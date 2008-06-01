insert into configuration_fields (plugin, name, field_type, description, default_value) VALUES (1, 'alists_every_presence', 2, 'Apply ALists to every presence', true);
insert into configuration_fields (plugin, name, field_type, description, default_value) VALUES (1, 'alists_members', 2, 'Apply ALists to members', false);
insert into configuration_fields (plugin, name, priority, field_type, description, default_value) values (1, 'devoice_no_vcard', 1, 2, 'Devoice participants without VCard', 'false');
insert into configuration_fields (plugin, name, priority, field_type, description, default_value) values (1, 'devoice_no_vcard_reason', 2, 1, 'No VCard devoice reason', 'Please fill your VCard and then say me !muc checkvcard');
