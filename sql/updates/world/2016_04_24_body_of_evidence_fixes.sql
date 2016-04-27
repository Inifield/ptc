-- Add missing Boulderfist Saboteur lines for quest Body of Evidence (Quest 9932)
DELETE FROM script_texts WHERE entry=-1900200 OR entry=-1900201 OR entry=-1900202 OR entry=-1900203; 
INSERT INTO script_texts (entry, content_default, content_loc1, content_loc2, content_loc3, content_loc4, content_loc5, content_loc6, content_loc7, content_loc8, sound, type, language, emote, comment) VALUES (-1900200, 'Shadow Council come for you Warmaul! We is mad at you!', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 'Boulderfist Saboteur');
INSERT INTO script_texts (entry, content_default, content_loc1, content_loc2, content_loc3, content_loc4, content_loc5, content_loc6, content_loc7, content_loc8, sound, type, language, emote, comment) VALUES (-1900201, 'I killed another Warmaul ogre! I am Shadow Council! Come get me.', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 'Boulderfist Saboteur');
INSERT INTO script_texts (entry, content_default, content_loc1, content_loc2, content_loc3, content_loc4, content_loc5, content_loc6, content_loc7, content_loc8, sound, type, language, emote, comment) VALUES (-1900202, 'You make dat sound pretty good. Sound like da orc dieded.', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 'Boulderfist Saboteur');
INSERT INTO script_texts (entry, content_default, content_loc1, content_loc2, content_loc3, content_loc4, content_loc5, content_loc6, content_loc7, content_loc8, sound, type, language, emote, comment) VALUES (-1900203, 'Dat\'s all of dem! Dese Warmaul be so mad! Hooray!', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 'Boulderfist Saboteur');

-- Make it so Kil'sorrow Invaders can't be attacked by players
UPDATE creature_template SET unit_flags=2 WHERE entry=18397;