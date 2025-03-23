#ifndef _cronus_agents_coder_h_
#define _cronus_agents_coder_h_

#include "cronus/sourceMap.h"
#include <memory>
#include <string>
#include <vector>

namespace cronus {
namespace agents {

/**
 * @class Coder
 * @brief Agent responsible for code generation, analysis, and refactoring
 * 
 * The Coder agent uses the source map to understand the codebase structure
 * and provides code-related functionality such as:
 * - Code generation based on natural language descriptions
 * - Code analysis and explanation
 * - Refactoring suggestions
 * - Bug identification and fixes
 */
class Coder {
public:
    /**
     * @brief Constructor
     * @param sourceMap Shared pointer to the source map
     */
    explicit Coder(std::shared_ptr<SourceMap> sourceMap);
    
    /**
     * @brief Generate code based on a natural language description
     * @param description The description of what code to generate
     * @param context Additional context like file references or tags
     * @return The generated code
     */
    std::string generateCode(const std::string& description, 
                            const std::vector<std::string>& context = {});
    
    /**
     * @brief Analyze and explain a piece of code
     * @param code The code to analyze
     * @return Explanation of the code
     */
    std::string explainCode(const std::string& code);
    
    /**
     * @brief Suggest refactoring for a piece of code
     * @param code The code to refactor
     * @param goal The goal of refactoring (e.g., "improve performance", "increase readability")
     * @return Refactored code with explanation
     */
    std::string suggestRefactoring(const std::string& code, const std::string& goal);
    
    /**
     * @brief Identify bugs in code
     * @param code The code to check for bugs
     * @return List of potential bugs with explanations
     */
    std::vector<std::string> identifyBugs(const std::string& code);
    
    /**
     * @brief Generate unit tests for a piece of code
     * @param code The code to test
     * @param framework The testing framework to use (default: auto-detect)
     * @return Generated unit tests
     */
    std::string generateTests(const std::string& code, const std::string& framework = "auto");

private:
    std::shared_ptr<SourceMap> m_sourceMap;
    
    /**
     * @brief Extract relevant context from the source map
     * @param description The description to analyze for context
     * @return Vector of relevant file paths and code snippets
     */
    std::vector<std::string> extractContext(const std::string& description);
    
    /**
     * @brief Format the generated code according to project standards
     * @param code The code to format
     * @param language The programming language
     * @return Formatted code
     */
    std::string formatCode(const std::string& code, const std::string& language);
};

} // namespace agents
} // namespace cronus

#endif // _cronus_agents_coder_h_
