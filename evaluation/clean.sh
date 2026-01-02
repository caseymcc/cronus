#!/bin/bash
# Clean evaluation results and optionally exercises

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXERCISES_DIR="${SCRIPT_DIR}/exercises"
RESULTS_DIR="${SCRIPT_DIR}/results"
CACHE_DIR="${SCRIPT_DIR}/.cache"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

usage() {
    cat << EOF
Usage: $0 [OPTIONS]

Clean evaluation data.

OPTIONS:
    --results       Clean evaluation results only (default)
    --exercises     Clean downloaded exercises (will need to re-download)
    --all           Clean everything (results, exercises, cache)
    --cache         Clean cache only
    -h, --help      Show this help message

EOF
}

CLEAN_RESULTS=false
CLEAN_EXERCISES=false
CLEAN_CACHE=false

if [ $# -eq 0 ]; then
    CLEAN_RESULTS=true
fi

while [[ $# -gt 0 ]]; do
    case $1 in
        --results)
            CLEAN_RESULTS=true
            shift
            ;;
        --exercises)
            CLEAN_EXERCISES=true
            shift
            ;;
        --cache)
            CLEAN_CACHE=true
            shift
            ;;
        --all)
            CLEAN_RESULTS=true
            CLEAN_EXERCISES=true
            CLEAN_CACHE=true
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
done

if [ "$CLEAN_RESULTS" = true ]; then
    if [ -d "${RESULTS_DIR}" ]; then
        log_info "Cleaning evaluation results..."
        rm -rf "${RESULTS_DIR}"
        log_info "Results cleaned"
    else
        log_warn "No results directory found"
    fi
fi

if [ "$CLEAN_CACHE" = true ]; then
    if [ -d "${CACHE_DIR}" ]; then
        log_info "Cleaning cache..."
        rm -rf "${CACHE_DIR}"
        log_info "Cache cleaned"
    else
        log_warn "No cache directory found"
    fi
fi

if [ "$CLEAN_EXERCISES" = true ]; then
    if [ -d "${EXERCISES_DIR}" ]; then
        log_warn "This will delete all downloaded Exercism exercises"
        read -p "Are you sure? (y/N) " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            log_info "Cleaning exercises..."
            rm -rf "${EXERCISES_DIR}"
            log_info "Exercises cleaned - run './evaluation/setup.sh' to re-download"
        else
            log_info "Exercises not cleaned"
        fi
    else
        log_warn "No exercises directory found"
    fi
fi

log_info "Clean complete"
