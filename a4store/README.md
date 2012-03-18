<!-- todo: link to local copies -->
<script src="http://yandex.st/highlightjs/6.1/highlight.min.js"></script>
<script>hljs.initHighlightingOnLoad();</script>
<link href="http://kevinburke.bitbucket.org/markdowncss/markdown.css" rel="stylesheet"></link>
<link rel="stylesheet" href="http://yandex.st/highlightjs/6.1/styles/default.min.css"></link>

# a4store

You have to make those histograms _again_. You've been asked how your distributions are affected by the new cut? It's easy, all you have to do is update the histogram variable definitions, the point you `new` the histograms and set their binning, the place you fill them and the place where you write them to your `TFile`. 

.. and then you have to update your plotting scripts to deal with the new condition. Then you just have to debug to correct for typos in you made whilst copy-pasting the existing code.

.. across three different files.

Okay, that's starting to sound like quite a bit of effort. What if the complete definition of your histogram looked like this?

    // C++
    Store.T<H1>("electron/pt")(100, 0, 100, "p_{T} [GeV]").fill(electron.pt());

## Easy to use but still fast

"Whoa there!", I hear you say. "Strings..", you murmur, "Function calls?!", I see your face twisting into shapes used to caracature high numbers on the medical pain scale.

Doesn't this kill my performance?

.. well, no, not really. Not with a modern optimising compiler (GCC>=4.3 will do). [Benchmarks here]. The whole thing gets inlined, and the second time the line gets hit, it is as if all of the initialization doesn't exist: it amounts to just the `H1::Fill` call. In addition, because `"electron/pt"` is a compiled in string (as opposed to a dynamic one) it lives in a special region of memory; this means that it can be identified purely by its pointer.

You pay something like 0-20%, but bear in mind that as a fraction of all the things your analysis does, an additional 0-20% on filling histograms will not have an important impact. [More detailed stats considering I/O here].

This buys you:
    
  - One line of code = one histogram
  - the ability to re-use the histogram definitions by putting them in functions
  - a meaningful directory hierarchy without having to write yet more code

## Back to the new cut

Where were we.. oh yes, there is a new cut, and you want to know how it affects the pt distribution.

    void plot_electron(ObjectStore Store, const Electron& electron) {
        Store.T<H1>("pt")(100, 0, 100, "p_{T} [GeV]").fill(electron.pt());
        // ... many more histograms here ...
    }
    
    void process_event(ObjectStore Store, const Event& event) {
        
        foreach (const Electron& electron, event.electrons()) {
            if (abs(electron.eta()) < 1.6)
                plot_electron(Store("electron/barrel"), electron);
            else
                plot_electron(Store("electron/endcap"), electron);
        }
    }
    
## Getting started

### Requirements

  - Compiler support for some C++0x features<br />
    (for `gcc` that means at least 4.3; 4.4 if you want to use initializer lists, and you'll need to compile with `-std=c++0x`)
    
    You'll need to compile your analysis with `-std=c++0x` if you're using `gcc`.

  - The a4store tarball or source

## Other features

  - Variable binning (requires initialiser lists support from your compiler)
    
        // C++
        Store.T<H1>("pt")({0, 10, 20, 40, 80, 160, 320}).fill(pt);
  
  - Dynamic store/histogram naming
  
        for (int i = 0; i < 10; i++)
            if (passcut(i))
                plot_electrons(Store("electron/passcut_", i), 
                               event);
        
  - Multiple dimensions
    
        // C++
        Store.T<H3>("ptetax")
            (10, 0, 10)(10, -2.5, 2.5)(10, 0, 1)
            .fill(pt, eta, x);
  
  - Setting titles
    
        // C++
        Store.T<H2>("pt_vs_eta")
            ("p_{T} as a function of eta")
            ({0, 10, 20, 40, 80, 160, 320}, "p_{T} [GeV]")
            (10, 0, 2.5, "|#eta|")
            .fill(pt, abs(eta));
  
  - Changing the Store base
  - Thread safety
    
    Blarg?
  
  - Standalone: Can be used outside of a4
  
