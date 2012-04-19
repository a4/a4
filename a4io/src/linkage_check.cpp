#include <a4/debug.h>

#include <link.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <vector>
#include <string>

class LinkageCheck {
public:
    struct stat _program_stat;
    std::vector<std::string> _young_libraries;

    static int linkage_check_callback(struct dl_phdr_info *info, size_t size, void *data)
    {
        LinkageCheck* self = reinterpret_cast<LinkageCheck*>(data);
        
        struct stat file_stat;
        if (stat(info->dlpi_name, &file_stat) != -1) {
            //DEBUG("File time: ", file_stat.st_mtime, " name: ", info->dlpi_name);
            if (self->_program_stat.st_mtime < file_stat.st_mtime) {
                self->_young_libraries.push_back(info->dlpi_name);
            } 
        }
        return 0;
    }

    LinkageCheck() {
        
        if (getenv("A4_SKIP_LINKAGECHECK") != NULL)
            return;
        
        if (stat("/proc/self/exe", &_program_stat) == -1) {
            WARNING("Couldn't stat self, runtime linkage check skipped");
            return;
        }
    
        dl_iterate_phdr(linkage_check_callback, this);
        
        if (_young_libraries.size()) {
            char prog[1024] = {0};
            ssize_t res = readlink("/proc/self/exe", prog, sizeof(prog));
            if (res == -1) {
                ERROR("Could not do linkage check since opening /proc/self/exe failed.");
                return;
            }
            prog[sizeof(prog)/sizeof(char) - 1] = 0;
            
        
            ERROR("The following libraries are younger than ", prog, ":");
            for (std::vector<std::string>::iterator i = _young_libraries.begin();
                  i != _young_libraries.end(); i++) {
                ERROR("  ", *i);
            }
                  
            FATAL("Linkage check failed. Recompile! Define A4_SKIP_LINKAGECHECK "
                  "if you really really really want to try anyway. "
                  "But don't complain if you get segfaults!");
        }
    }
};

LinkageCheck linkage_check;
