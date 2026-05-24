#include "EventProducer.h"
#include <userver/utils/uuid4.hpp>
#include <chrono>

namespace medical::infrastructure {

EventProducer::EventProducer(userver::rabbitmq::ClientPtr rabbit_client)
    : rabbit_(std::move(rabbit_client)) {}

void EventProducer::Publish(const std::string& routingKey, const userver::formats::json::Value& payload) {
    userver::formats::json::ValueBuilder event;
    event["eventId"] = userver::utils::generators::GenerateUuid();
    event["eventType"] = routingKey;
    event["timestamp"] = std::chrono::system_clock::now().time_since_epoch().count();
    event["payload"] = payload;
    
    rabbit_->Publish("medical.events", routingKey, userver::formats::json::ToString(event.ExtractValue()));
}

void EventProducer::PublishUserRegistered(int userId, const std::string& login, const std::string& email) {
    userver::formats::json::ValueBuilder payload;
    payload["userId"] = userId;
    payload["login"] = login;
    payload["email"] = email;
    Publish("user.registered", payload.ExtractValue());
}

void EventProducer::PublishUserLoggedIn(int userId, const std::string& login, const std::string& ip) {
    userver::formats::json::ValueBuilder payload;
    payload["userId"] = userId;
    payload["login"] = login;
    payload["ipAddress"] = ip;
    Publish("user.logged_in", payload.ExtractValue());
}

void EventProducer::PublishPatientCreated(int patientId, const std::string& fullName, const std::string& snils) {
    userver::formats::json::ValueBuilder payload;
    payload["patientId"] = patientId;
    payload["fullName"] = fullName;
    payload["snils"] = snils;
    Publish("patient.created", payload.ExtractValue());
}

void EventProducer::PublishMedicalRecordCreated(int recordId, const std::string& code, int patientId, int doctorId) {
    userver::formats::json::ValueBuilder payload;
    payload["recordId"] = recordId;
    payload["code"] = code;
    payload["patientId"] = patientId;
    payload["doctorId"] = doctorId;
    Publish("record.created", payload.ExtractValue());
}

void EventProducer::PublishDiagnosisAdded(int recordId, int patientId, const std::string& code, const std::string& desc) {
    userver::formats::json::ValueBuilder payload;
    payload["recordId"] = recordId;
    payload["patientId"] = patientId;
    payload["diagnosisCode"] = code;
    payload["diagnosisDescription"] = desc;
    Publish("diagnosis.added", payload.ExtractValue());
}

}  // namespace medical::infrastructure
