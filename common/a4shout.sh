#! /usr/bin/env bash

# set crash if anything fails, or upon encountering unset variables
set -u
set -e

# Configurables
API_URL=https://github.com/api/v2/yaml
if [[ -z "${REPO-}" ]]; then
    REPO=JohannesEbke/a4
fi

LAST_ERROR_FILE="${XDG_CACHE_HOME-${HOME}/.cache}/a4/last-error"
ISSUE_TITLE="a4shout report"
ISSUES_URL="https://github.com/${REPO}/issues"

# Test if stdout is a terminal we can use color on
if tty -s <&1; then STDOUT_IS_TTY=true; else STDOUT_IS_TTY=false; fi
# Safe workaround for ancient HTTPS certificates
CA_CERT="$( cd "$( dirname "$0" )" && pwd )"/github-ca/curl-ca-bundle-github.crt

function color {
    local COLOR=$1
    shift
    
    if [[ "$COLOR" == "red" ]]; then COLOR=31;
    elif [[ "$COLOR" == "green" ]]; then COLOR=32;
    elif [[ "$COLOR" == "blue" ]]; then COLOR=34;
    else COLOR=1; fi
    
    if $STDOUT_IS_TTY; then
        #echo $'\033[1;'${COLOR}'m'$@$'\033[0m'
        echo -n $'\033[1;'${COLOR}'m'"$@"$'\033[0m'
    else
        echo "$@"
    fi
}

function bold { color bold "$@"; }

function debug {
    # Emit debugging information if DEBUG is set
    if test -n "${DEBUG:+x}"; then echo "[$(color blue DEBUG)]" "$@"; fi
}

function error { echo "[$(color red ERROR)]" "$@" >&2; }
function inform { echo "[$(color green INFO)]" "$@"; }
function about { echo "[$(color blue ABOUT)]" "$@" >&2; }

function die {
    error "$@"
    debug "Exiting.."
    exit 1
}

function usage {

    about "\"a4shout\" creates a github issue at "
    about "  $(bold ${ISSUES_URL}) "
    about "from either the input provided on $(bold stdin),"
    about "the file specified in the $(bold arguments), or"
    about "the contents of the file (if it exists):"
    about "  \"$(bold ${LAST_ERROR_FILE}\")."
    inform "Usage:"
    inform "  $ a4shout.sh --last [issue title...] # (if ${LAST_ERROR_FILE} exists)"
    inform "  $ a4shout.sh filename [issue title...]"
    inform "  $ program 2>1 | ./a4shout - [issue title...]"
    inform "  $ a4shout more issue_number [filename|-] [comment text...]"

    if [[ $USER_FAILED == true || $TOKEN_FAILED == true ]]; then 
        error "Please configure your github.user and github.token!"
        inform "Create an account in seconds here if you don't have one:"
        inform "  https://github.com/signup/free"
        inform "Then run:"
        inform "  git config --global github.user your_github_username"
        inform "  git config --global github.token 0123456789yourf0123456789token"
        inform "  chmod go-rw ${HOME}/.gitconfig # Keep your token safe from prying eyes!"
        error See http://help.github.com/set-your-user-name-email-and-github-token/
    fi
    
    exit 1
}

USER_FAILED=false
TOKEN_FAILED=false

USER="$(git config --get github.user || echo __FAILED__)"
if [[ "$USER" == "__FAILED__" ]]; then USER_FAILED=true; fi
TOKEN="$(git config --get github.token || echo __FAILED__)"
if [[ "$TOKEN" == "__FAILED__" ]]; then TOKEN_FAILED=true; fi

if [[ $TOKEN_FAILED == "false" ]]; then
    if [[ -n "$(chmod -fc go-r "${HOME}/.gitconfig")" ]]; then
        error "Your ${HOME}/.gitconfig was world readable, exposing your tokens."
        error "This has been fixed."
    fi
fi

if [[ $TOKEN_FAILED == "true" || $USER_FAILED == "true" ]]; then
    usage
fi

if [[ -z "${1-}" ]]; then
    usage
fi

if [[ "${1}" == "--test" ]]; then
    export REPO="pwaller/test"
    export DEBUG=1
        
    inform "--- Test posting self"
    $0 $0 "Test posting self"
    inform "--- Test posting stdin"
    echo issue content goes here | $0 - "Test posting stdin"
    inform "--- Test last-error"
    echo last-error content goes here > ${LAST_ERROR_FILE}
    RESULT="$($0 --last "Last error test" | grep -oE 'Issue [0-9]+ created' | grep -oE '[0-9]+' || true)"
    inform "--- Test commenting"
    debug "Issue number? $RESULT"
    $0 more ${RESULT} "This is a test comment."
    inform ".. Finished testing"
    exit
fi

# Is the user extending an existing issue?
ISSUE_NUMBER=
USING_LAST_ERROR_FILE=false

if [[ -n "${1-}" ]]; then
    if [[ "${1}" == "more" ]]; then
        if [[ -z "${2-}" ]]; then
            error "'more' specified, but no issue number!"
            usage
        fi
        ISSUE_NUMBER="$2"
        shift 2
    fi

    # File specified on commandline
    INPUT="${1}"
    if [[ "$INPUT" == "--last" ]]; then
        INPUT="${LAST_ERROR_FILE}"
        USING_LAST_ERROR_FILE=true
    fi
    if [[ ! -e "${INPUT}" && "${INPUT}" != "-" ]]; then
        die "Filename \"${INPUT}\" does not exist!"; 
    fi
    CONTENTS="$(cat "$INPUT")"
    shift
    if [[ -n "${1-}" ]]; then
        ISSUE_TITLE="$@"
    fi
else
    # Nothing. Don't know what to do!
    error "Either specify a filename or provide some stdin."
    usage
fi

# Format contents using awk to give indentation
CONTENTS="$(echo -n "$CONTENTS" | awk '{ print "    "$0 }')"

function github {
    CONTENTS_VAR=$1
    shift
    EXPECTED_RESPONSE=$1
    shift
    CALL=$(echo $1 | sed "s|{REPO}|${REPO}|")
    shift
    local RESULT="$(
        curl --write-out "%{http_code}" \
             --cacert "${CA_CERT}" \
             -F "login=${USER}" \
             -F "token=${TOKEN}" \
             "$@" \
             ${API_URL}/${CALL} \
             2>/dev/null)"
    RESPONSE="$(echo "$RESULT" | head -n-1)"
    RESPONSE_CODE=$(echo "$RESULT" | tail -n1)
    if [[ "$RESPONSE_CODE" != "$EXPECTED_RESPONSE" ]]; then
        error "Bad response from github ${API_URL}/${CALL}"
        if [[ $RESPONSE_CODE == "401" ]]; then
            error "Bad authorization. Is your \"git config --global github.token\" correct?"
            error "See: http://help.github.com/set-your-user-name-email-and-github-token/"
        fi;
        die "Expected HTTP code ${EXPECTED_RESPONSE} but got \"${RESPONSE_CODE}\""
    fi;
    debug "Reading into \"$CONTENTS_VAR\""
    export $CONTENTS_VAR="$RESPONSE"
}

function get_yaml {
    local VAR="$1"
    shift
    echo "$@" | grep -E "^ +$VAR"':.*$' | sed -r "s/^ +$VAR: +(.*)$/\1/"
}

if [[ -z "$ISSUE_NUMBER" ]]; then
    BODY="$(
        echo "This issue was automatically created by a4shout.sh" &&
        echo &&
        echo "The contents of ${INPUT}:"
        echo &&
        echo "${CONTENTS}"
    )"

    debug "Creating new issue"
    github ISSUE 201 "issues/open/{REPO}" -F "title=${ISSUE_TITLE}" -F "body=${BODY}"
    
    ISSUE_NUMBER=$(get_yaml number "${ISSUE}")
    URL=$(get_yaml html_url "${ISSUE}")

    inform "Issue ${ISSUE_NUMBER} created: ${URL}"
    inform "Please edit the issue to add more context, or use"
    inform "  $(bold "$ a4shout more ${ISSUE_NUMBER} [file] [comment]")"
    inform "to add additional information."
else
    debug "Commenting on existing issue"
    
    COMMENT="$(
        echo "Comment made through a4shout.sh" &&
        echo "${ISSUE_TITLE}" &&
        echo "The contents of ${INPUT}:"
        echo &&
        echo "${CONTENTS}"
    )"
    github COMMENT_RESULT 201 "issues/comment/{REPO}/${ISSUE_NUMBER}" -F "comment=${COMMENT}"
    
    COMMENT_ID="$(get_yaml id "${COMMENT_RESULT}")"
    COMMENT_URL="${ISSUES_URL}/${ISSUE_NUMBER}#issuecomment-${COMMENT_ID}"
    
    inform "Comment posted to ${COMMENT_URL}"
fi

#debug "Labelling issue"
#github RESULT 200 issues/label/add/{REPO}/${LABELNAME}/${ISSUE_NUMBER}

if $USING_LAST_ERROR_FILE; then
    NEW_FILENAME="${LAST_ERROR_FILE}-issue-${ISSUE_NUMBER}"
    mv $LAST_ERROR_FILE $NEW_FILENAME
    inform "Moved ${LAST_ERROR_FILE} to ${NEW_FILENAME}"
fi

debug "Success!"
