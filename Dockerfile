FROM php:7.2-apache

RUN apt update

RUN docker-php-source extract

RUN apt install nano -y

RUN apt install rsyslog --fix-missing -y

COPY config/piof-iast.ini /usr/local/etc/php/conf.d

COPY config/rsyslog.conf /etc/rsyslog.conf


COPY src /opt/piof-iast

WORKDIR /opt/piof-iast/

RUN ./scripts/buildall.sh
