-- Create tables
-- Table: casual_subscriptions

DROP TABLE IF EXISTS casual_subscriptions;
CREATE TABLE casual_subscriptions
(
  id serial NOT NULL,
  number_rentals integer,
  duration_minutes numeric,
  subscriber_id character varying(32),
  subscription_type character varying(128),
  zip character varying(16),
  status character varying(16),
  activation_start_date timestamp without time zone,
  activation_end_date timestamp without time zone,
  gc_friendly_name character varying(128),
  bikes integer,
  CONSTRAINT casual_subscriptions_id PRIMARY KEY (id )
)
WITH (
  OIDS=FALSE
);

-- Table: rentals

DROP TABLE IF EXISTS rentals;
CREATE TABLE rentals
(
  id serial NOT NULL,
  rental_id character varying(16),
  account_number character varying(32),
  subscriber_id character varying(32),
  start_date timestamp without time zone,
  start_station character varying(128),
  start_termninal character varying(16),
  end_date timestamp without time zone,
  end_station character varying(128),
  end_termninal character varying(16),
  duration_seconds numeric,
  bike_id character varying(16),
  gender character varying(8),
  zip character varying(16),
  subscription_type_a character varying(128),
  subscription_type_b character varying(128),
  birth_date date,
  CONSTRAINT rentals_id PRIMARY KEY (id )
)
WITH (
  OIDS=FALSE
);

-- Table: subscribers

DROP TABLE IF EXISTS subscribers;
CREATE TABLE subscribers
(
  id serial NOT NULL,
  gender character varying(8),
  number_rentals integer,
  duration_minutes numeric,
  account_number character varying(32),
  subscriber_id character varying(32),
  subscription_type character varying(128),
  activation_status character varying(16),
  city character varying(128),
  state character varying(32),
  zip character varying(16),
  status character varying(16),
  birth_date date,
  start_date timestamp without time zone,
  end_date timestamp without time zone,
  CONSTRAINT subscribers_id PRIMARY KEY (id )
)
WITH (
  OIDS=FALSE
);

-- Table: stations

DROP TABLE IF EXISTS stations;
CREATE TABLE stations
(
  id serial NOT NULL,
  terminal_id character varying(16),
  common_name character varying(128),
  station character varying(128),
  lat numeric,
  lon numeric,
  install_date date,
  CONSTRAINT stations_id PRIMARY KEY (id )
)
WITH (
  OIDS=FALSE
);
