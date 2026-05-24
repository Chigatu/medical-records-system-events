# CQRS Design — Medical Records System

## 1. Применимость CQRS

**CQRS (Command Query Responsibility Segregation)** — разделение операций на команды (write) и запросы (read).

В нашей системе CQRS **применим частично**:
- **Чтение:** поиск пациентов, история записей — частые запросы, требуют оптимизации
- **Запись:** создание пациентов, записей — редкие операции, но критичны к консистентности

## 2. Разделение на команды и запросы

### Команды (Write Model)

| Команда | Сервис | База данных |
|---------|--------|-------------|
| `RegisterUser` | AuthService | PostgreSQL (`users`) |
| `CreatePatient` | PatientService | PostgreSQL (`patients`) |
| `CreateMedicalRecord` | RecordService | PostgreSQL (`medical_records`) |
| `UpdateDiagnosis` | RecordService | PostgreSQL (`medical_records`) |

### Запросы (Read Model)

| Запрос | Сервис | База данных |
|--------|--------|-------------|
| `SearchUsers` | UserService | Redis (кеш) |
| `SearchPatients` | PatientService | MongoDB (`patients`) |
| `GetPatientRecords` | RecordService | MongoDB (`medical_records`) |
| `GetDiagnosisStats` | StatisticsService | MongoDB (агрегации) |

## 3. Синхронизация моделей через события

```
┌─────────────┐     Событие      ┌─────────────┐
│  Write DB   │─────────────────▶│  Read DB    │
│ (PostgreSQL)│                  │  (MongoDB)  │
└─────────────┘                  └─────────────┘
       │                                │
       │ Команды                        │ Запросы
       ▼                                ▼
┌─────────────┐                  ┌─────────────┐
│  Command    │                  │  Query      │
│  Handlers   │                  │  Handlers   │
└─────────────┘                  └─────────────┘
```

### Поток синхронизации:

1. **Команда** `CreatePatient` → запись в PostgreSQL (`patients`)
2. Публикуется событие `PatientCreated`
3. **SearchIndexService** получает событие → создает документ в MongoDB (`patients`)
4. **Запрос** `SearchPatients` → чтение из MongoDB (с поисковым индексом)

## 4. Преимущества CQRS в нашей системе

| Преимущество | Описание |
|--------------|----------|
| **Раздельная оптимизация** | Write-модель оптимизирована для транзакций, Read-модель — для поиска |
| **Масштабирование** | Read-сервисы можно масштабировать независимо |
| **Гибкость БД** | PostgreSQL для write (транзакции), MongoDB для read (гибкие запросы) |
| **Кеширование** | Read-модель можно кешировать в Redis |

## 5. Eventual Consistency

- **Задержка синхронизации:** < 1 секунда (обычно < 100ms)
- **Гарантии:** At-least-once доставка событий
- **Идемпотентность:** Consumer проверяет `eventId` перед обработкой
- **Конфликты:** Разрешаются по timestamp (последнее изменение побеждает)

## 6. Структура Read/Write моделей

### Write Model (PostgreSQL) — нормализованная

```sql
-- Пример: Patient (write)
CREATE TABLE patients (
    id SERIAL PRIMARY KEY,
    first_name VARCHAR(100) NOT NULL,
    last_name VARCHAR(100) NOT NULL,
    snils VARCHAR(20) UNIQUE,
    ...
);
```

### Read Model (MongoDB) — денормализованная

```json
// Пример: Patient (read)
{
  "_id": ObjectId,
  "patientId": 15,
  "fullName": "Сидоров Александр Иванович",
  "snils": "123-456-789 01",
  "age": 46,
  "lastRecord": {  // денормализованные данные последней записи
    "diagnosisCode": "J06.9",
    "date": "2026-05-24"
  }
}
```
