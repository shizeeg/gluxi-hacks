DROP TABLE IF EXISTS conference_log CASCADE;
CREATE TABLE conference_log (
  id SERIAL,

  conference_id int NOT NULL,
  datetime timestamp NOT NULL default now(), 
  private boolean NOT NULL,  
  nick_id int NOT NULL,
  action_type int NOT NULL,
  message varchar(500) NOT NULL,
  params varchar(100) NULL,

  PRIMARY KEY(id),
  FOREIGN KEY(conference_id) REFERENCES conferences(id) ON DELETE CASCADE,
  FOREIGN KEY(nick_id) REFERENCES conference_nicks(id) ON DELETE CASCADE
);

