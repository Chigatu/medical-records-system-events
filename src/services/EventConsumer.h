#pragma once
#include <string>
#include <functional>
#include <userver/rabbitmq/client.hpp>

namespace medical::infrastructure {

class EventConsumer {
public:
    using Callback = std::function<void(const std::string& eventType, const std::string& payload)>;
    
    explicit EventConsumer(userver::rabbitmq::ClientPtr rabbit_client);
    
    void Subscribe(const std::string& queue, const std::string& bindingKey, Callback callback);
    void StartConsuming();

private:
    userver::rabbitmq::ClientPtr rabbit_;
};

}  // namespace medical::infrastructure
