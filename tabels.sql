-- Table: public.users_log_pus

DROP TABLE IF EXISTS public.users_log_pus;

CREATE TABLE IF NOT EXISTS public.users_log_pus
(
    id bigint NOT NULL GENERATED ALWAYS AS IDENTITY ( INCREMENT 1 START 1 MINVALUE 1 MAXVALUE 9223372036854775807 CACHE 1 ),
    login character varying COLLATE pg_catalog."default",
    password character varying COLLATE pg_catalog."default",
    CONSTRAINT users_log_pus_pkey PRIMARY KEY (id)
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
	
