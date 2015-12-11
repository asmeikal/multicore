

create or replace view tempisequenziali
as
select piattaforma, implementazione, versione, lpass, nmaster, avg(tempo) as "tempo"
from test
where nthread = 1
group by piattaforma, implementazione, versione, lpass, nmaster;

create or replace view tempi as
select piattaforma, implementazione, versione, lpass, nthread, nmaster, avg(tempo) as "tempo", extract(epoch from greatest(max(tempo) - avg(tempo), avg(tempo) - min(tempo))) as "dev", cast(avg(pcpu) as decimal(6,2))::text || '%' as "pcpu"
from test
group by piattaforma, implementazione, versione, lpass, nthread, nmaster;

create or replace view speedup as
select t1.piattaforma, t1.implementazione, t1.versione, t1.lpass, t1.nthread, t1.nmaster, (extract(epoch from t2.tempo) / extract(epoch from t1.tempo)) as "speedup"
from tempi as t1
join tempisequenziali as t2
on t1.piattaforma = t2.piattaforma
and t1.implementazione = t2.implementazione
and t1.versione = t2.versione
and t1.lpass = t2.lpass
and t1.nmaster = t2.nmaster;

create or replace function getdata(text, integer, integer, integer, integer)
returns table (threads integer, tempo interval, "tempo (s)" decimal(6,2), "deviazione (s)" decimal(6,2), speedup decimal(6,2), "uso cpu" text) as
$BODY$
declare
p alias for $1;
i alias for $2;
l alias for $3;
m alias for $4;
v alias for $5;
begin
	return query select s.nthread, t.tempo, cast(extract(epoch from t.tempo) as decimal(6,2)), cast(t.dev as decimal(6,2)), cast(s.speedup as decimal(6,2)), t.pcpu
	from speedup as s
	natural join tempi as t
	where s.piattaforma = p
	and s.implementazione = i
	and s.lpass = l
	and s.nmaster = m
	and s.versione = v
	order by s.nthread;
end
$BODY$
language plpgsql;

create or replace view testlist
as
select implementazione, piattaforma, versione, lpass, count(*) as "# test", min(nthread)::text || '-' || max(nthread)::text as "# thread", array_agg(distinct(nmaster)) as "nmaster"
from test group by piattaforma, implementazione, versione, lpass
order by implementazione, piattaforma, lpass, versione;

create or replace view tempi_finocchi
as
select t.difficulty, t.nthread, t.tempo, (cast(extract(epoch from t.tempo) as decimal(10,2))) as "tempo (s)", (extract(epoch from (select tempo from finocchi where nthread = 1 and difficulty = t.difficulty)) / extract(epoch from t.tempo)) as "speedup"
from finocchi as t
order by t.difficulty, t.nthread;

