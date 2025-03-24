#ifndef _cronus_model_h_
#define _cronus_model_h_

#include "hermes/hermes.h"
#include <string>
#include <functional>
#include <memory>
#include <vector>

namespace cronus
{

/**
 * @class Model
 * @brief Handles interactions with language models through Hermes
 *
 * This class provides a unified interface for interacting with various
 * language models through the Hermes library, handling model selection,
 * API key management, and request formatting.
 */
class Model
{
public:
    /**
     * @brief Constructor
     * @param modelName The name of the model to use
     * @param provider The provider of the model
     * @param maxTokens The maximum number of tokens the model can handle
     */
    Model(const std::string &modelName, const std::string &provider);

    /**
     * @brief Generate a completion from the model
     * @param messages Vector of messages for the conversation
     * @param streaming Whether to use streaming mode
     * @param callback Callback function for streaming responses
     * @return Generated text response
     */
    std::string generate(
        const std::vector<hermes::Message> &messages,
        bool streaming=false,
        std::function<void(const std::string &)> callback=nullptr);

    /**
     * @brief Get the maximum token limit for the current model
     * @return Maximum token limit
     */
    size_t getMaxTokens() const;

    /**
     * @brief Get the current model name
     * @return Model name
     */
    std::string getModelName() const;

private:
    std::string m_currentModel;
    std::string m_currentProvider;
    size_t m_maxTokens;
};

} // namespace cronus

#endif // _cronus_model_h_
