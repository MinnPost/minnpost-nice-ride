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
  start_terminal character varying(16),
  end_date timestamp without time zone,
  end_station character varying(128),
  end_terminal character varying(16),
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
SELECT AddGeometryColumn('stations', 'location_geom', 4326, 'POINT', 2);


-- Table: routes

DROP TABLE IF EXISTS routes;
CREATE TABLE routes
(
  id serial NOT NULL,
  terminal_id_start character varying(16),
  terminal_id_end character varying(16),
  CONSTRAINT routes_id PRIMARY KEY (id )
)
WITH (
  OIDS=FALSE
);
SELECT AddGeometryColumn('routes', 'start_geom', 4326, 'POINT', 2);
SELECT AddGeometryColumn('routes', 'end_geom', 4326, 'POINT', 2);
SELECT AddGeometryColumn('routes', 'route_geom', 4326, 'LINESTRING', 2);

-- Table: average_day
DROP TABLE IF EXISTS average_day;
CREATE TABLE average_day
(
  id serial NOT NULL,
  start_time time without time zone,
  end_time time without time zone,
  total integer,
  average numeric,
  CONSTRAINT average_day_id PRIMARY KEY (id )
)
WITH (
  OIDS=FALSE
);

-- Table: average_all_days
DROP TABLE IF EXISTS average_all_days;
CREATE TABLE average_all_days
(
  id serial NOT NULL,
  day date,
  start_time time without time zone,
  end_time time without time zone,
  total integer,
  difference numeric,
  CONSTRAINT average_all_days_id PRIMARY KEY (id )
)
WITH (
  OIDS=FALSE
);
