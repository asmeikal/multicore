

CREATE OR REPLACE VIEW tempisequenziali AS
SELECT 		piattaforma,
			implementazione,
			versione,
			lpass,
			nmaster,
			AVG(tempo) AS "tempo"
FROM	 	test
WHERE 		nthread = 1
GROUP BY 	piattaforma,
			implementazione,
			versione,
			lpass,
			nmaster;

CREATE OR REPLACE VIEW tempi AS
SELECT 		piattaforma,
			implementazione,
			versione,
			lpass,
			nthread,
			nmaster,
			AVG(tempo) AS "tempo",
			EXTRACT(epoch FROM GREATEST(MAX(tempo) - AVG(tempo), AVG(tempo) - MIN(tempo))) AS "dev",
			CAST(AVG(pcpu) AS decimal(6,2))::text || '%' AS "pcpu"
FROM 		test
GROUP BY 	piattaforma,
			implementazione,
			versione,
			lpass,
			nthread,
			nmaster;

CREATE OR REPLACE VIEW speedup AS
SELECT 		t1.piattaforma,
			t1.implementazione,
			t1.versione,
			t1.lpass,
			t1.nthread,
			t1.nmaster,
			EXTRACT(epoch FROM t2.tempo) / EXTRACT(epoch FROM t1.tempo) AS "speedup"
FROM 		tempi AS t1
JOIN 		tempisequenziali AS t2
	ON 		t1.piattaforma = t2.piattaforma
	AND 	t1.implementazione = t2.implementazione
	AND 	t1.versione = t2.versione
	AND 	t1.lpass = t2.lpass
	AND 	t1.nmaster = t2.nmaster;

CREATE OR REPLACE VIEW testlist AS
SELECT 		implementazione,
			piattaforma,
			versione,
			lpass,
			COUNT(*) AS "# test",
			MIN(nthread)::text || '-' || MAX(nthread)::text AS "# thread",
			ARRAY_AGG(DISTINCT(nmaster)) AS "nmaster"
FROM 		test
GROUP BY 	piattaforma,
			implementazione,
			versione,
			lpass
ORDER BY 	implementazione,
			piattaforma,
			lpass,
			versione;

CREATE OR REPLACE VIEW tempi_finocchi AS
SELECT 		t.difficulty,
			t.nthread,
			t.tempo,
			CAST(EXTRACT(epoch FROM t.tempo) AS decimal(10,2)) AS "tempo (s)",
			EXTRACT(epoch FROM (SELECT tempo FROM finocchi WHERE nthread = 1 AND difficulty = t.difficulty)) /
			EXTRACT(epoch FROM t.tempo) AS "speedup"
FROM 		finocchi AS t
ORDER BY 	t.difficulty, t.nthread;

CREATE OR REPLACE FUNCTION getdata(text, integer, integer, integer, integer)
RETURNS TABLE (	threads integer,
				tempo interval,
				"tempo (s)" decimal(6,2),
				"deviazione (s)" decimal(6,2),
				speedup decimal(6,2),
				"uso cpu" text
				) AS
$BODY$
DECLARE
p ALIAS FOR $1;
i ALIAS FOR $2;
l ALIAS FOR $3;
m ALIAS FOR $4;
v ALIAS FOR $5;
BEGIN
	RETURN QUERY
	SELECT 			s.nthread,
					t.tempo,
					CAST(EXTRACT(epoch FROM t.tempo) AS decimal(6,2)),
					CAST(t.dev AS decimal(6,2)),
					CAST(s.speedup AS decimal(6,2)),
					t.pcpu
	FROM 			speedup AS s
	NATURAL JOIN 	tempi AS t
	WHERE 			s.piattaforma = p
		AND 		s.implementazione = i
		AND 		s.lpass = l
		AND 		s.nmaster = m
		AND 		s.versione = v
	ORDER BY 		s.nthread;
END
$BODY$
LANGUAGE plpgsql;
