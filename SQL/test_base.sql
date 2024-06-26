-- Database: stock_market

DROP DATABASE IF EXISTS test_stock_market;

CREATE DATABASE test_stock_market
    WITH
    OWNER = postgres
    ENCODING = 'UTF8'
    LC_COLLATE = 'ru_RU.UTF-8'
    LC_CTYPE = 'ru_RU.UTF-8'
    TABLESPACE = pg_default
    CONNECTION LIMIT = -1
    IS_TEMPLATE = False;

-- SCHEMA: public

DROP SCHEMA IF EXISTS public ;

CREATE SCHEMA IF NOT EXISTS public
    AUTHORIZATION postgres;

COMMENT ON SCHEMA public
    IS 'standard public schema';

GRANT ALL ON SCHEMA public TO PUBLIC;

GRANT ALL ON SCHEMA public TO postgres;

--- Table: public.users_log_pus

DROP TABLE IF EXISTS public.users_log_pus;

CREATE TABLE IF NOT EXISTS public.users_log_pus
(
    id bigint NOT NULL GENERATED ALWAYS AS IDENTITY ( INCREMENT 1 START 1 MINVALUE 1 MAXVALUE 9223372036854775807 CACHE 1 ),
    login character varying COLLATE pg_catalog."default",
    password character varying COLLATE pg_catalog."default",
    online boolean DEFAULT false,
    CONSTRAINT users_log_pus_pkey PRIMARY KEY (id)
)
WITH (
    OIDS = FALSE
)
TABLESPACE pg_default;

ALTER TABLE IF EXISTS public.users_log_pus
    OWNER to postgres;

-- Table: public.user_balance

DROP TABLE IF EXISTS public.user_balance;

CREATE TABLE IF NOT EXISTS public.user_balance
(
    id bigint NOT NULL GENERATED ALWAYS AS IDENTITY ( INCREMENT 1 START 1 MINVALUE 1 MAXVALUE 9223372036854775807 CACHE 1 ),
    user_id bigint NOT NULL,
    balance numeric DEFAULT 0,
    usd_count bigint DEFAULT 0,
    CONSTRAINT user_balance_pkey PRIMARY KEY (id),
    CONSTRAINT fk_users_log_pus_id FOREIGN KEY (user_id)
        REFERENCES public.users_log_pus (id) MATCH SIMPLE
        ON UPDATE NO ACTION
        ON DELETE NO ACTION
)

TABLESPACE pg_default;

ALTER TABLE IF EXISTS public.user_balance
    OWNER to postgres;

-- Table: public.request_purchase_sale

DROP TABLE IF EXISTS public.request_purchase_sale;

CREATE TABLE IF NOT EXISTS public.request_purchase_sale
(
    id bigint NOT NULL GENERATED ALWAYS AS IDENTITY ( INCREMENT 1 START 1 MINVALUE 1 MAXVALUE 9223372036854775807 CACHE 1 ),
    user_id bigint NOT NULL,
    request_date timestamp without time zone NOT NULL DEFAULT timezone('utc'::text, now()),
    dollar_price numeric NOT NULL,
    dollars_count bigint NOT NULL,
    sale boolean DEFAULT false,
    CONSTRAINT request_purchase_sale_pkey PRIMARY KEY (id),
    CONSTRAINT fk_users_log_pus_id FOREIGN KEY (user_id)
        REFERENCES public.users_log_pus (id) MATCH SIMPLE
        ON UPDATE NO ACTION
        ON DELETE NO ACTION
)

TABLESPACE pg_default;

ALTER TABLE IF EXISTS public.request_purchase_sale
    OWNER to postgres;

-- Table: public.transaction_history

DROP TABLE IF EXISTS public.transaction_history;

CREATE TABLE IF NOT EXISTS public.transaction_history
(
    id bigint NOT NULL GENERATED ALWAYS AS IDENTITY ( INCREMENT 1 START 1 MINVALUE 1 MAXVALUE 9223372036854775807 CACHE 1 ),
    buyer_id bigint NOT NULL,
    seller_id bigint NOT NULL,
    deal_date timestamp without time zone NOT NULL DEFAULT timezone('utc'::text, now()),
    dollar_price numeric NOT NULL,
    dollars_count bigint NOT NULL,
    CONSTRAINT transaction_history_pkey PRIMARY KEY (id),
    CONSTRAINT fk_users_log_pus_id1 FOREIGN KEY (buyer_id)
        REFERENCES public.users_log_pus (id) MATCH SIMPLE
        ON UPDATE NO ACTION
        ON DELETE NO ACTION,
    CONSTRAINT fk_users_log_pus_id2 FOREIGN KEY (seller_id)
        REFERENCES public.users_log_pus (id) MATCH SIMPLE
        ON UPDATE NO ACTION
        ON DELETE NO ACTION
)

TABLESPACE pg_default;

ALTER TABLE IF EXISTS public.transaction_history
    OWNER to postgres;
