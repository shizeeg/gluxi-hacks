alter table conference_alists add child_id int NOT NULL DEFAULT 0 after `value`;
alter table conference_alists drop key conference_id;
alter TABLE conference_alists add key (conference_id,list,matcher,test,value, child_id);
