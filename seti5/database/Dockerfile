# Используем официальный образ MySQL
FROM mysql:8.0

# Устанавливаем переменные окружения (можно переопределить при запуске)
ENV MYSQL_ROOT_PASSWORD=zaglotus
ENV MYSQL_DATABASE=doxed
ENV MYSQL_USER=default_user
ENV MYSQL_PASSWORD=zaglotus

# Копируем кастомные конфигурационные файлы
COPY ./my.cnf /etc/mysql/conf.d/
COPY ./init.sql /docker-entrypoint-initdb.d/

# Открываем порт MySQL
EXPOSE 3306

# Используем стандартный entrypoint из базового образа
CMD ["mysqld"]