DELETE FROM conference_jids where temporary=true;
DELETE FROM conference_jids where jid in (SELECT jid FROM (select jid, count(jid) as cnt from conference_jids group by conference_id, jid) as tbl where cnt > 1);
UPDATE conference_jids set jid=LOWER(jid);

... cleanup nicks/jids here ...

ALTER TABLE conference_jids add UNIQUE(conference_id, jid);
ALTER TABLE conference_nicks add UNIQUE(conference_id, nick, jid);
