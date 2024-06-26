# market
client-server app with postgre and boost  
(РЕАЛИЗАЦИЯ С QT В ОТДЕЛЬНОЙ ВЕТКЕ)
# Необходимые библиотеки
  Для сборки проекта потребуются следующие библиотеки:
  1. Boost (для установки необходимо ввести команду `apt install libboost-all-dev libboost-thread-dev`).
  2. PostgreSql (для установки необходимо ввести команду `apt install libpq-dev python-dev postgresql postgresql-contrib`).
  3. Google Tests (для установки необходимо ввести команду `apt install libgtest-dev`).
  4. Cmake (для установки необходимо ввести команду `apt install cmake`).
# Порядок сборки
  Сборка проверена на ОС Linux Astra 17.
  Есть несколько вариантов сборки:
  1. Открыть CMakeLists.txt в Qt.
  2. Собрать вручную:  
      Пример директории
<pre>
├──build_dir
└────project_root
      └── CMakeLists.txt      
</pre>  

Сборка проекта
    
```
cd build_dir  
cmake ../project_root  
cmake --build .  
```

# Проект состоит из 3 подпрограмм:
  1. Сервер
  2. Клиент
  3. Тесты сервера

# Запуск проекта
  1. Необходимо `Создать` базу данных(путь к скрипту создания БД `SQL/base.sql`).
  2. Запустить сервер `Server` в папке исполняемых файлов директории `build_dir` (Если БД создана, и запущен сервер postgresql, то он свяжется с БД).
  3. Запустить клиентов `Client` в папке исполняемых файлов директории `build_dir` (Если сервер запущен, свяжется с ним).
     
# Запуск тестов
  1. Необходимо `Создать` базу данных(путь к скрипту создания БД `SQL/test_base.sql`).
  2. Запустить тесты 'TestServer'.

# Использование программы
  1. После запуска клиента необходимо зарегестрироваться(если ранее пользователь не был зарегестрирован) или войти в свой профиль.
  2. Начать торговать.
# Возможности клиента:
  1. Запросить баланс.
  2. Добавить запрос на продажу.
  3. Добавить запрос на покупку.
  4. Посмотреть активные запросы.
  5. Посмотреть `мои` активные запросы.
  6. Отменить `мой` запрос.
  7. Посмотреть `мои` совершенные сделки.
  8. Посмотреть котировку доллара.
  9. Выйти.
