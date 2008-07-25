alter table conference_alists add child_id int NOT NULL DEFAULT 0;
alter table conference_alists DROP CONSTRAINT conference_alists_conference_id_key;
alter TABLE conference_alists ADD UNIQUE (conference_id,list,matcher,test,value, child_id);

