#!/bin/bash
# Main evaluation runner script
# Runs agent evaluation against Exercism exercises
# Automatically manages Docker container for evaluation

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
EXERCISES_DIR="${SCRIPT_DIR}/exercises"
RESULTS_DIR="${SCRIPT_DIR}/results"
SCRIPTS_DIR="${SCRIPT_DIR}/scripts"
CONFIG_FILE="${SCRIPT_DIR}/config.json"

# Docker configuration
CONTAINER_NAME="cronus_eval"
IMAGE_NAME="cronus-evaluation"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default values
LANGUAGE=""
EXERCISE=""
DRY_RUN=false
VERBOSE=false
REBUILD_DOCKER=false
NO_DOCKER=false

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

log_debug() {
    if [ "$VERBOSE" = true ]; then
        echo -e "${BLUE}[DEBUG]${NC} $1"
    fi
}

usage() {
    cat << EOF
Usage: $0 [OPTIONS]

Run Cronus agent evaluation against Exercism exercises.
By default, automatically runs in Docker container with all dependencies installed.

OPTIONS:
    -l, --language LANG     Run evaluation for specific language (cpp, python, javascript)
    -e, --exercise NAME     Run specific exercise by name
    -d, --dry-run          Show what would be evaluated without running
    -v, --verbose          Enable verbose output
    -r, --rebuild          Rebuild Docker image and exit (does not run evaluation)
    --no-docker            Run directly on host (not recommended, requires all deps)
    -h, --help             Show this help message

EXAMPLES:
    # Run all evaluations (automatically in Docker)
    $0

    # Run Python evaluations only
    $0 --language python

    # Run specific exercise
    $0 --language python --exercise hello-world

    # Rebuild Docker image only (does not run evaluation)
    $0 --rebuild

    # Dry run to see what would be executed
    $0 --dry-run

    # Verbose output
    $0 --language cpp --verbose

NOTE:
    This script automatically manages the Docker container.
    - Builds image if not present
    - Runs evaluation inside container
    - All dependencies (pytest, jest, g++, etc.) are available in the container
    
    The --rebuild flag ONLY rebuilds the Docker image and exits.
    To rebuild and then run evaluation, use two commands:
      $0 --rebuild
      $0 --language python
    
    Use --no-docker only if you have all dependencies installed on your host system.
EOF
}

# Detect if we're already inside Docker
IN_DOCKER=false
if [ -f /.dockerenv ] || grep -q docker /proc/1/cgroup 2>/dev/null; then
    IN_DOCKER=true
fi

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -l|--language)
            LANGUAGE="$2"
            shift 2
            ;;
        -e|--exercise)
            EXERCISE="$2"
            shift 2
            ;;
        -d|--dry-run)
            DRY_RUN=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -r|--rebuild)
            REBUILD_DOCKER=true
            shift
            ;;
        --no-docker)
            NO_DOCKER=true
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            log_error "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
done

# If not already in Docker and not disabled, manage Docker container
if [ "$IN_DOCKER" = false ] && [ "$NO_DOCKER" = false ]; then
    log_info "Managing Docker container for evaluation..."
    
    # Check if image needs to be built or rebuilt
    if [ "$REBUILD_DOCKER" = true ] || ! docker image inspect ${IMAGE_NAME} >/dev/null 2>&1; then
        if [ "$REBUILD_DOCKER" = true ]; then
            log_info "Rebuilding evaluation Docker image..."
        else
            log_info "Building evaluation Docker image (first time)..."
        fi
        docker build -t ${IMAGE_NAME} -f "${SCRIPT_DIR}/Dockerfile" "${SCRIPT_DIR}"
        
        # If --rebuild was explicitly requested, stop here
        if [ "$REBUILD_DOCKER" = true ]; then
            log_info "Docker image rebuilt successfully"
            log_info "Run without --rebuild to execute evaluation"
            exit 0
        fi
    fi
    
    # Remove existing container if running
    docker rm -f ${CONTAINER_NAME} 2>/dev/null || true
    
    # Get host network interface IP (for accessing host services)
    HOST_IP=$(ip route get 1 | awk '{print $7;exit}')
    
    log_info "Running evaluation in Docker container..."
    log_info "Host IP: ${HOST_IP} (for accessing llama.cpp server)"
    
    # Build arguments array to pass to container
    DOCKER_ARGS=("./run_evaluation.sh")
    
    if [ -n "$LANGUAGE" ]; then
        DOCKER_ARGS+=("--language" "$LANGUAGE")
    fi
    
    if [ -n "$EXERCISE" ]; then
        DOCKER_ARGS+=("--exercise" "$EXERCISE")
    fi
    
    if [ "$DRY_RUN" = true ]; then
        DOCKER_ARGS+=("--dry-run")
    fi
    
    if [ "$VERBOSE" = true ]; then
        DOCKER_ARGS+=("--verbose")
    fi
    
    # Add --no-docker to prevent infinite recursion inside container
    DOCKER_ARGS+=("--no-docker")
    
    # Run container with:
    # - Project mounted at /app
    # - Network access to host
    # - Same user permissions
    exec docker run --rm -it \
        --name ${CONTAINER_NAME} \
        --network host \
        --add-host=host.docker.internal:host-gateway \
        -v "${PROJECT_ROOT}:/app" \
        -w /app/evaluation \
        -e HOST_IP="${HOST_IP}" \
        ${IMAGE_NAME} \
        "${DOCKER_ARGS[@]}"
fi

# We're now either inside Docker or running with --no-docker
if [ "$IN_DOCKER" = true ]; then
    log_info "Running inside Docker container"
elif [ "$NO_DOCKER" = true ]; then
    log_warn "Running directly on host (--no-docker mode)"
    log_warn "Ensure all dependencies are installed: python3, pytest, jest, g++, etc."
fi

# Check if exercises are downloaded
if [ ! -d "${EXERCISES_DIR}" ]; then
    log_error "Exercises not found. Run './evaluation/setup.sh' first."
    exit 1
fi

# Check if Python is available (needed for evaluation scripts)
if ! command -v python3 &> /dev/null; then
    log_error "python3 is required but not installed."
    exit 1
fi

# Check if required Python packages are installed
log_info "Checking Python dependencies..."
MISSING_DEPS=()

if ! python3 -c "import requests" 2>/dev/null; then
    MISSING_DEPS+=("requests")
fi

if ! python3 -c "import websocket" 2>/dev/null; then
    MISSING_DEPS+=("websocket-client")
fi

if [ ${#MISSING_DEPS[@]} -gt 0 ]; then
    log_error "Missing Python dependencies: ${MISSING_DEPS[*]}"
    log_error ""
    log_error "These packages should be installed in the Docker image."
    log_error "Please rebuild the evaluation Docker image:"
    log_error "  cd evaluation && docker build -t cronus-evaluation -f Dockerfile ."
    log_error ""
    log_error "Or install manually inside container:"
    log_error "  pip3 install ${MISSING_DEPS[*]}"
    exit 1
fi

log_info "All Python dependencies are installed"

# Check if config file exists
if [ ! -f "${CONFIG_FILE}" ]; then
    log_error "Config file not found: ${CONFIG_FILE}"
    exit 1
fi

# Create results directory with timestamp
TIMESTAMP=$(date +"%Y-%m-%d_%H-%M-%S")
RESULTS_RUN_DIR="${RESULTS_DIR}/archive/${TIMESTAMP}"
mkdir -p "${RESULTS_RUN_DIR}"

# Create/update latest symlink
rm -f "${RESULTS_DIR}/latest"
ln -s "archive/${TIMESTAMP}" "${RESULTS_DIR}/latest"

log_info "Starting evaluation run: ${TIMESTAMP}"
log_info "Results will be saved to: ${RESULTS_RUN_DIR}"

# Determine which languages to evaluate
if [ -n "${LANGUAGE}" ]; then
    LANGUAGES=("${LANGUAGE}")
else
    LANGUAGES=(cpp python javascript)
fi

# Check if Cronus executable is built (required for evaluation)
log_info "Checking if Cronus server is built..."
CRONUS_EXECUTABLE=$(python3 -c "import json; print(json.load(open('${CONFIG_FILE}'))['agent'].get('cronus_executable', 'build/linux_x64_debug/server/cronus/cronus'))" 2>/dev/null || echo "build/linux_x64_debug/server/cronus/cronus")

# Convert to absolute path
if [[ ! "$CRONUS_EXECUTABLE" = /* ]]; then
    CRONUS_EXECUTABLE="${PROJECT_ROOT}/${CRONUS_EXECUTABLE}"
fi

if [ ! -f "${CRONUS_EXECUTABLE}" ]; then
    log_error "Cronus executable not found at: ${CRONUS_EXECUTABLE}"
    log_error ""
    log_error "Please build Cronus first:"
    log_error "  cd ${PROJECT_ROOT}"
    log_error "  ./run_local.sh ./generate.sh"
    log_error "  ./run_local.sh ninja -C build/linux_x64_debug"
    log_error ""
    log_error "The evaluation system requires a built Cronus server."
    exit 1
fi

if [ ! -x "${CRONUS_EXECUTABLE}" ]; then
    log_error "Cronus executable is not executable: ${CRONUS_EXECUTABLE}"
    log_error "Run: chmod +x ${CRONUS_EXECUTABLE}"
    exit 1
fi

log_info "Found Cronus executable: ${CRONUS_EXECUTABLE}"
log_info "Cronus server will be started automatically by evaluation script"

# Run evaluation for each language
for lang in "${LANGUAGES[@]}"; do
    log_info "Evaluating ${lang} exercises..."
    
    if [ ! -d "${EXERCISES_DIR}/${lang}" ]; then
        log_warn "Exercises for ${lang} not found, skipping..."
        continue
    fi
    
    # Create language-specific results directory
    LANG_RESULTS_DIR="${RESULTS_RUN_DIR}/${lang}"
    mkdir -p "${LANG_RESULTS_DIR}"
    
    # Build evaluation command
    EVAL_CMD="python3 ${SCRIPTS_DIR}/evaluate_exercise.py"
    EVAL_CMD="${EVAL_CMD} --language ${lang}"
    EVAL_CMD="${EVAL_CMD} --exercises-dir ${EXERCISES_DIR}/${lang}"
    EVAL_CMD="${EVAL_CMD} --results-dir ${LANG_RESULTS_DIR}"
    EVAL_CMD="${EVAL_CMD} --config ${CONFIG_FILE}"
    
    if [ -n "${EXERCISE}" ]; then
        EVAL_CMD="${EVAL_CMD} --exercise ${EXERCISE}"
    fi
    
    if [ "$VERBOSE" = true ]; then
        EVAL_CMD="${EVAL_CMD} --verbose"
    fi
    
    if [ "$DRY_RUN" = true ]; then
        log_info "Would run: ${EVAL_CMD}"
    else
        log_debug "Running: ${EVAL_CMD}"
        if ${EVAL_CMD}; then
            log_info "Evaluation for ${lang} completed successfully"
        else
            EXIT_CODE=$?
            log_error "Evaluation for ${lang} failed with exit code ${EXIT_CODE}"
            
            # Check common failure reasons
            if [ ${EXIT_CODE} -eq 1 ]; then
                log_error ""
                log_error "Common causes:"
                log_error "  - Cronus server failed to start (check build status)"
                log_error "  - Port 9000 already in use (check with: netstat -tlnp | grep 9000)"
                log_error "  - Cronus server crashed during evaluation"
                log_error "  - Configuration error in ${CONFIG_FILE}"
                log_error ""
                log_error "Check the output above for specific error messages."
            fi
            
            # Don't continue with other languages if this one failed
            log_warn "Stopping evaluation due to errors"
            exit ${EXIT_CODE}
        fi
    fi
done

# Generate summary report
if [ "$DRY_RUN" = false ]; then
    log_info "Generating summary report..."
    python3 "${SCRIPTS_DIR}/generate_report.py" \
        --results-dir "${RESULTS_RUN_DIR}" \
        --output "${RESULTS_RUN_DIR}/summary.html" \
        --format html
    
    python3 "${SCRIPTS_DIR}/generate_report.py" \
        --results-dir "${RESULTS_RUN_DIR}" \
        --output "${RESULTS_RUN_DIR}/summary.json" \
        --format json
    
    log_info "Evaluation complete!"
    log_info "Cronus server has been stopped automatically"
    echo ""
    echo "Results saved to: ${RESULTS_RUN_DIR}"
    echo "Summary report: ${RESULTS_RUN_DIR}/summary.html"
    echo "JSON report: ${RESULTS_RUN_DIR}/summary.json"
    echo ""
    
    # Display quick summary
    if [ -f "${RESULTS_RUN_DIR}/summary.json" ]; then
        python3 -c "
import json
import sys
with open('${RESULTS_RUN_DIR}/summary.json', 'r') as f:
    data = json.load(f)
    print('Quick Summary:')
    for lang, stats in data.get('languages', {}).items():
        total = stats.get('total', 0)
        passed = stats.get('passed', 0)
        print(f'  {lang}: {passed}/{total} passed ({passed*100//total if total > 0 else 0}%)')
" 2>/dev/null || true
    fi
else
    log_info "Dry run complete - no changes made"
fi
