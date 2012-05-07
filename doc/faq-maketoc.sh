
#!/usr/bin/env bash

read -r -d '' PROGRAM <<'EOF'
from sys import stdin
from string import punctuation
for line in stdin:
    line = line.strip()
    link = line.lower().replace(" ", "-")
    for punct in punctuation.replace("-", ""):
        link = link.replace(punct, "")
    print "* [{0}](#{1})".format(line, link)
EOF

grep -B1 -E "^--" FAQ.md | grep -vE "^--" | python -c "$PROGRAM"
