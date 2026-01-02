#!/bin/bash
# Setup script for Exercism evaluation exercises
# Downloads Exercism exercise repositories and prepares them for evaluation

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXERCISES_DIR="${SCRIPT_DIR}/exercises"
CACHE_DIR="${SCRIPT_DIR}/.cache"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if git is available
if ! command -v git &> /dev/null; then
    log_error "git is required but not installed."
    exit 1
fi

# Create directories if they don't exist
mkdir -p "${EXERCISES_DIR}"
mkdir -p "${CACHE_DIR}"

# Function to clone or update a repository
clone_or_update_repo() {
    local repo_url="$1"
    local target_dir="$2"
    local repo_name="$3"
    
    if [ -d "${target_dir}/.git" ]; then
        log_info "${repo_name} already exists, updating..."
        cd "${target_dir}"
        git fetch origin
        git reset --hard origin/main || git reset --hard origin/master
        cd - > /dev/null
    else
        log_info "Cloning ${repo_name}..."
        git clone --depth 1 "${repo_url}" "${target_dir}"
    fi
}

# Function to create exercise index
create_exercise_index() {
    local lang="$1"
    local exercises_path="${EXERCISES_DIR}/${lang}/exercises"
    local index_file="${CACHE_DIR}/${lang}_index.json"
    
    log_info "Creating exercise index for ${lang}..."
    
    if [ ! -d "${exercises_path}" ]; then
        log_warn "Exercise path not found: ${exercises_path}"
        return 1
    fi
    
    # Create a simple JSON index of available exercises
    echo "[" > "${index_file}"
    local first=true
    
    for exercise_dir in "${exercises_path}"/practice/*/; do
        if [ -d "${exercise_dir}" ]; then
            exercise_name=$(basename "${exercise_dir}")
            
            # Skip if .meta/config.json doesn't exist
            if [ ! -f "${exercise_dir}/.meta/config.json" ]; then
                continue
            fi
            
            if [ "$first" = true ]; then
                first=false
            else
                echo "," >> "${index_file}"
            fi
            
            echo "  {" >> "${index_file}"
            echo "    \"name\": \"${exercise_name}\"," >> "${index_file}"
            echo "    \"path\": \"${exercise_dir}\"," >> "${index_file}"
            
            # Try to extract difficulty if available in config
            if [ -f "${exercise_dir}/.meta/config.json" ]; then
                difficulty=$(grep -o '"difficulty"[[:space:]]*:[[:space:]]*[0-9]*' "${exercise_dir}/.meta/config.json" | grep -o '[0-9]*$' || echo "5")
                echo "    \"difficulty\": ${difficulty}" >> "${index_file}"
            else
                echo "    \"difficulty\": 5" >> "${index_file}"
            fi
            
            echo "  }" >> "${index_file}"
        fi
    done
    
    echo "]" >> "${index_file}"
    
    local exercise_count=$(grep -c '"name"' "${index_file}" || echo "0")
    log_info "Indexed ${exercise_count} ${lang} exercises"
}

# Download C++ exercises
log_info "Setting up C++ exercises..."
clone_or_update_repo \
    "https://github.com/exercism/cpp.git" \
    "${EXERCISES_DIR}/cpp" \
    "C++"
create_exercise_index "cpp"

# Download Python exercises
log_info "Setting up Python exercises..."
clone_or_update_repo \
    "https://github.com/exercism/python.git" \
    "${EXERCISES_DIR}/python" \
    "Python"
create_exercise_index "python"

# Download JavaScript exercises
log_info "Setting up JavaScript exercises..."
clone_or_update_repo \
    "https://github.com/exercism/javascript.git" \
    "${EXERCISES_DIR}/javascript" \
    "JavaScript"
create_exercise_index "javascript"

# Create summary
log_info "Setup complete!"
echo ""
echo "Exercise repositories downloaded to: ${EXERCISES_DIR}"
echo "Exercise indexes created in: ${CACHE_DIR}"
echo ""
echo "Available exercises:"
for lang in cpp python javascript; do
    if [ -f "${CACHE_DIR}/${lang}_index.json" ]; then
        count=$(grep -c '"name"' "${CACHE_DIR}/${lang}_index.json" || echo "0")
        echo "  - ${lang}: ${count} exercises"
    fi
done
echo ""
log_info "Run './evaluation/run_evaluation.sh' to start evaluations"
