#!/usr/bin/env python

from sys import path, stderr
path.append("common")

import pyinotify
import pycsp
import subprocess

from pycsp import process, ChannelPoisonException, InputGuard, TimeoutGuard, Spawn, Channel, poison, retire, AltSelect
from time import sleep


import logging
from logging import getLogger, basicConfig; log = getLogger("autocompile")
basicConfig(level=logging.WARNING)
getLogger("pyinotify").setLevel(logging.ERROR)

def exception():
    from sys import exc_info
    import traceback
    a, b, c = exc_info()
    print >>stderr, "Hit exception..", a, b, c
    traceback.print_exc(file=sys.stdout)
    raise

@process
def waiter(p, done):
    """
    Wait until process `p` has completed, then write the result to the `done`
    channel.
    """
    try:
        result = p.wait()
        log.debug("Finished waiting on process {0}, result: {1}".format(p.pid, result))
    finally:
        done(result)
        poison(done)
        log.debug("waiter exited ({0})".format(result))

def attempt_graceful_exit(p, done, patience):
    """
    Attempt a graceful exit on process `p`. `done` is the reader end of a channel
    which is written to when `p` exits, and `patience` is a number of seconds
    to wait for `p` to exit before sending in the big guns.
    """
    # Hello? Anybody there?
    if p.poll() is not None:
        return
        
    log.debug("Process is still going, signal it to gracefully exit")
    try:
        p.terminate()
    except OSError:
        pass # Ignore race condition between poll and terminate
    
    log.debug("Waiting for graceful exit response.. {0}".format(p))
    
    try:
        # Kill if child process doesn't respond within `patience` seconds
        channel, msg = AltSelect(
            InputGuard(done),
            TimeoutGuard(patience),
        )
    except:
        exception()
    finally:
        poison(done)
    
    if channel == done:
        log.debug("Got graceful exit")
    else:
        log.debug("Getting gun out")
    
    if p.poll() is None:
        try:
            p.kill() # *BLAM*
        except OSError:
            pass # Ignore race condition between poll and terminate
    
    log.debug("Process is done {0}".format(p))
    

@process
def runner(what, interrupt, finished, patience=2):
    """
    Executes `what` in a shell. If `interrupt` is poison by someone else, runner
    attempts to kill the subprocess, otherwise runner waits until the process exits.
    
    If the subprocess does not exit gracefully after SIGTERM, `patience` seconds
    are waited before sending SIGKILL.    
    
    When the process has finally exited (one way or another), `finished` is 
    written to.
    """
    
    try:
        #p = subprocess.Popen(["./waf", "build"]) # what) #, shell=True)
        p = subprocess.Popen(what)
    except:
        print "EXCEPTION"
        exception()
        
    log.debug("Spooling up ({0}) '{1}'".format(p.pid, what))
    
    done_channel = Channel("Done")
    done = done_channel.reader()
    
    Spawn(waiter(p, done_channel.writer()))
    
    try:
        # Wait until either the process is finished or we are interrupted
        channel, msg = AltSelect(
            InputGuard(interrupt), 
            InputGuard(done),
        )
        
        if channel == done:
            log.debug("Clean exit")
            return
        
    except ChannelPoisonException:
        log.debug("runner poisoned")
        
    except:
        from sys import exc_info
        print >>stderr, "Hit exception2..", exc_info()
        raise
        
    finally:
        poison(interrupt)
        
        try:
            attempt_graceful_exit(p, done, patience)
        finally:
            log.debug("Exited {0}".format(p))
            finished(True)
            #poison(finished)
            log.debug("Runner exited ({0})".format(p.pid))

def make_graceful_runner(what, grace_period=1):
    """
    Returns a function ('x') which when called, (re)starts a timeout of `grace_period` 
    seconds. If `grace_period` elapses before the x is called, `what` is run at
    the shell. If `what` is already running when 'x' is called, the process is
    killed and the timeout resumes again.
    """
        
    signal_channel = Channel("signal")
    receive_signal = signal_channel.reader()
    
    timeout = TimeoutGuard(grace_period)
    
    def wait_grace_period():
        while True:
            channel, msg = AltSelect(
                InputGuard(receive_signal),
                timeout,
            )
            if channel == receive_signal:
                continue
            else:
                break
    
    def start_interruptable_run():
        interrupt_channel, finished_channel = Channel("Interrupt"), Channel("Finished")
        interrupt = interrupt_channel.writer()
        wait_until_finished = finished_channel.reader()
        
        try:
            Spawn(runner(what, interrupt_channel.reader(), finished_channel.writer()))
            
            try:
                channel, msg = AltSelect(
                    InputGuard(receive_signal),
                    InputGuard(wait_until_finished),
                )
            except ChannelPoisonException:
                print " --- compile aborted ---"
                if receive_signal.channel.ispoisoned:
                    return
                        
            if channel == receive_signal:
                print " === new change, interrupting compile ==="
                log.debug("Recieved another signal, restarting")
                interrupt(True)
                poison(interrupt)
                
                log.debug(" Waiting [[[[")
                wait_until_finished()
                log.debug("             ]]]]]")
                # Go again!
                return True
                
            elif channel == wait_until_finished:
                print r" \\\ compilation finished ///"
                log.debug("Graceful finish.")
                
        except:
            exception()
        finally:
            poison(interrupt, wait_until_finished)
    
    graceful_runner_exit_channel = Channel("GRE")
    wait_for_graceful_runner_exit = graceful_runner_exit_channel.reader()
    graceful_runner_exit = graceful_runner_exit_channel.writer()
        
    @process
    def graceful_runner():
        try:
            while True:
                log.debug("Waiting for trigger <<<<")
                receive_signal()
                log.debug("                    >>>> trigger fired")
                wait_grace_period()
                while start_interruptable_run():
                    log.debug("Restarting interruptable run")
                
        except ChannelPoisonException:
            pass
        except:
            exception()            
        finally:
            log.debug("Graceful_runner exiting")
            poison(receive_signal)
            graceful_runner_exit(True)
            log.debug("Graceful_runner exited")
            
    Spawn(graceful_runner())
    signal = signal_channel.writer()
    def run():
        try:
            signal(True)
        except ChannelPoisonException:
            raise RuntimeError("runner already died")
            
    def finish():
        poison(signal)
        wait_for_graceful_runner_exit()
    
    return run, finish

def test():

    finish = lambda: None
    try:
        #what = "trap '' TERM; for i in {1..100}; do sleep 0.1 && echo ignoring TERM $i; done"
        #what = "for i in {1..10}; do echo $i; sleep 0.5; done;"
        what = "echo '@@@@@@@@@@@@@@@@@@'  && ./waf build"
        
        grace = 1
        run, finish = make_graceful_runner(what, grace)
        
        log.debug("Calling run (not enough time to execute)")
        run()
        sleep(grace / 2)
        log.debug("Calling run (with enough time)")
        run()
        sleep(grace * 1.1)
        log.debug("Oops, we hit recompile again but waf wasn't done yet")
        
        log.debug("Calling run (causing restart)")
        run()
        log.debug(" {{{{{{{{ Wait long enough for it to run")
        sleep(5)
        log.debug(" }}}}}}}} finish waiting")
        
        log.debug("Calling run (causing counter reset, should block until we're ready to run again)")
        run()
        log.debug(".... now we're unblocked and ready to run again")
        log.debug("Calling run again, twice without enough time")
        run()
        sleep(grace * 0.9)
        log.debug("Calling run..")
        run()
        sleep(grace * 1.1)
        log.debug("And again, this time with enough time..")
        run()
        #sleep(5)
    #except:
        #exception()
    finally:
        log.debug("FINISHING")
        finish()
        log.error("DONE")
    

def main(args):

    MOD_MASK = (pyinotify.IN_MODIFY | pyinotify.IN_MOVED_TO | pyinotify.IN_CREATE |
                pyinotify.IN_DELETE | pyinotify.IN_DELETE_SELF |
                pyinotify.IN_MOVE_SELF | pyinotify.IN_ATTRIB)
    
    finish = lambda: None
    try:
    
        # "./waf build"
        
        run, finish = make_graceful_runner(args, 0.1)
        
        def file_change(event):
            if not event.mask & MOD_MASK: return
            if ("config.h" in event.pathname or
                "conf_check" in event.pathname or
                ".unittest-gtest" in event.pathname or
                event.pathname.endswith(".pb.h") or
                event.pathname.endswith(".pb.cc")):
                return
                
            if (event.pathname.endswith(".cpp") or
                event.pathname.endswith(".h")):
                print "File changed: ", event.pathname
                run()

        from os import getcwd

        wm = pyinotify.WatchManager()
        notifier = pyinotify.Notifier(wm, file_change)
        wm.add_watch(getcwd(), pyinotify.ALL_EVENTS, rec=True)
        print "Will execute '{0}' when changes are made..".format(" ".join(args))
        notifier.loop()
        
        
    except KeyboardInterrupt:
        print "C^. Quitting."
        raise
    finally:
        print "Exited notifier loop"
        log.debug("FINISHING")
        finish()
        


if __name__ == "__main__":
    from sys import argv
    main(argv[1:])
