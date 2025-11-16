#ifndef _cronus_chatMessage_h_
#define _cronus_chatMessage_h_

namespace cronus
{

enum class ChatType
{
    Message,
    Completion,
    Log
};

enum class Role
{
    User,
    Bot,
    Error,
    Warning,
    Log
};

struct ChatMessage
{
    ChatMessage(ChatType type, Role role, const std::string &content) :
        type(type),
        role(role),
        content(content)
    {
    }

    ChatType type;
    Role role;

    std::string content;
};

    
} // namespace cronus

#endif // _cronus_chatMessage_h_