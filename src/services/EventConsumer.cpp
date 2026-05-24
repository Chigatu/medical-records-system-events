#include "EventConsumer.h"
#include <iostream>

namespace medical::infrastructure {

EventConsumer::EventConsumer(userver::rabbitmq::ClientPtr rabbit_client)
    : rabbit_(std::move(rabbit_client)) {}

void EventConsumer::Subscribe(const std::string& queue, const std::string& bindingKey, Callback callback) {
    rabbit_->DeclareQueue(queue);
    rabbit_->BindQueue(queue, "medical.events", bindingKey);
    rabbit_->Consume(queue, [callback](const std::string& message) {
        auto json = userver::formats::json::FromString(message);
        std::string eventType = json["eventType"].As<std::string>();
        std::string payload = userver::formats::json::ToString(json["payload"]);
        callback(eventType, payload);
    });
}

void EventConsumer::StartConsuming() {
    std::cout << "EventConsumer started, waiting for messages..." << std::endl;
}

}  // namespace medical::infrastructure
