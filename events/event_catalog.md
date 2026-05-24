# Event Catalog — Medical Records System

## 1. UserRegistered

| Поле | Значение |
|------|----------|
| **Название** | `UserRegistered` |
| **Описание** | Новый пользователь (врач) зарегистрирован в системе |
| **Producer** | AuthService (`POST /api/auth/register`) |
| **Consumers** | AuditService, NotificationService |
| **Exchange** | `medical.events` (topic) |
| **Routing Key** | `user.registered` |
| **Гарантии** | At-least-once |

### Payload (JSON):
*```*json
{
  "eventId": "evt-uuid-12345",
  "eventType": "UserRegistered",
  "timestamp": "2026-05-24T12:00:00Z",
  "payload": {
    "userId": 42,
    "login": "dr.new_doctor",
    "email": "new_doctor@hospital.ru",
    "firstName": "Новый",
    "lastName": "Доктор"
  }
}
*```*

---

## 2. UserLoggedIn

| Поле | Значение |
|------|----------|
| **Название** | `UserLoggedIn` |
| **Описание** | Пользователь успешно аутентифицировался |
| **Producer** | AuthService (`POST /api/auth/login`) |
| **Consumers** | AuditService |
| **Exchange** | `medical.events` (topic) |
| **Routing Key** | `user.logged_in` |
| **Гарантии** | At-least-once |

### Payload (JSON):
*```*json
{
  "eventId": "evt-uuid-12346",
  "eventType": "UserLoggedIn",
  "timestamp": "2026-05-24T12:05:00Z",
  "payload": {
    "userId": 42,
    "login": "dr.new_doctor",
    "ipAddress": "192.168.1.100"
  }
}
*```*

---

## 3. PatientCreated

| Поле | Значение |
|------|----------|
| **Название** | `PatientCreated` |
| **Описание** | Новый пациент зарегистрирован |
| **Producer** | PatientService (`POST /api/patients`) |
| **Consumers** | AuditService, SearchIndexService |
| **Exchange** | `medical.events` (topic) |
| **Routing Key** | `patient.created` |
| **Гарантии** | At-least-once |

### Payload (JSON):
*```*json
{
  "eventId": "evt-uuid-12347",
  "eventType": "PatientCreated",
  "timestamp": "2026-05-24T12:10:00Z",
  "payload": {
    "patientId": 15,
    "firstName": "Александр",
    "lastName": "Сидоров",
    "snils": "123-456-789 01",
    "createdBy": 42
  }
}
*```*

---

## 4. MedicalRecordCreated

| Поле | Значение |
|------|----------|
| **Название** | `MedicalRecordCreated` |
| **Описание** | Создана новая медицинская запись |
| **Producer** | RecordService (`POST /api/medical-records`) |
| **Consumers** | AuditService, StatisticsService, CacheInvalidationService |
| **Exchange** | `medical.events` (topic) |
| **Routing Key** | `record.created` |
| **Гарантии** | At-least-once |

### Payload (JSON):
*```*json
{
  "eventId": "evt-uuid-12348",
  "eventType": "MedicalRecordCreated",
  "timestamp": "2026-05-24T12:15:00Z",
  "payload": {
    "recordId": 100,
    "code": "MED-20260524-00001",
    "patientId": 15,
    "doctorId": 42,
    "diagnosisCode": "J06.9",
    "diagnosisDescription": "Острая респираторная инфекция"
  }
}
*```*

---

## 5. DiagnosisAdded

| Поле | Значение |
|------|----------|
| **Название** | `DiagnosisAdded` |
| **Описание** | Поставлен новый диагноз пациенту |
| **Producer** | RecordService (`POST /api/medical-records`) |
| **Consumers** | StatisticsService |
| **Exchange** | `medical.events` (topic) |
| **Routing Key** | `diagnosis.added` |
| **Гарантии** | At-least-once |

### Payload (JSON):
*```*json
{
  "eventId": "evt-uuid-12349",
  "eventType": "DiagnosisAdded",
  "timestamp": "2026-05-24T12:15:00Z",
  "payload": {
    "recordId": 100,
    "patientId": 15,
    "diagnosisCode": "J06.9",
    "diagnosisDescription": "Острая респираторная инфекция"
  }
}
*```*

---

## Exchange и Routing

| Exchange | Тип | Routing Keys |
|----------|------|--------------|
| `medical.events` | topic | `user.*`, `patient.*`, `record.*`, `diagnosis.*` |

## Очереди (Queues)

| Очередь | Binding Key | Consumer |
|---------|-------------|----------|
| `audit.queue` | `#` (все события) | AuditService |
| `notification.queue` | `user.registered` | NotificationService |
| `statistics.queue` | `record.*`, `diagnosis.*` | StatisticsService |
| `cache_invalidation.queue` | `record.created` | CacheInvalidationService |
| `search_index.queue` | `patient.created` | SearchIndexService |
