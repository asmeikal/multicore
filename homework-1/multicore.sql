
CREATE DOMAIN Piattaforma
	AS text
	CHECK ((value = 'i5')
		OR (value = 'iMac')
		OR (value = 'Pi'));

CREATE TABLE JavaVersions (
	id serial PRIMARY KEY,
	name varchar(100) UNIQUE NOT NULL
);

CREATE DOMAIN Implementazione
AS integer
CHECK ((value = 1) OR (value = 2));

CREATE TABLE Test (
	id serial PRIMARY KEY,
	piattaforma Piattaforma not null,
	implementazione Implementazione not null,
	versione integer not null references javaVersions(id),
	nthread integer not null,
	tempo interval not null,
	pcpu integer,
	nmaster integer not null,
	lpass integer not null,
	npass integer not null,
	check (tempo > '0 seconds'),
	check (nthread > 0),
	check (lpass > 0),
	check (npass > 0),
	check (nmaster > 0),
	check (pcpu is null or pcpu > 0)
);

CREATE DOMAIN difficolta
AS text
CHECK ((value = 'easy') OR (value = 'medium') OR (value = 'hard'));

CREATE TABLE finocchi (
	id serial primary key,
	piattaforma piattaforma not null,
	implementazione implementazione not null,
	versione integer not null references javaversions(id),
	nthread integer not null,
	tempo interval not null,
	difficulty difficolta not null,
	nmaster integer not null,
	check (tempo > '0 seconds'),
	check (nthread > 0),
	check (nmaster > 0)
);



