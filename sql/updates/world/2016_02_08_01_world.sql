-- Pathing for Bloodcursed Naga Entry: 17713 'TDB FORMAT' 
SET @NPC := 63485;
SET @PATH := @NPC * 10;
UPDATE `creature` SET `spawndist`=0,`MovementType`=2,`position_x`=-1928.789,`position_y`=-12869.43,`position_z`=86.3521 WHERE `guid`=@NPC;
DELETE FROM `creature_addon` WHERE `guid`=@NPC;
INSERT INTO `creature_addon` (`guid`,`path_id`,`mount`,`bytes1`,`bytes2`,`emote`,`auras`) VALUES (@NPC,@PATH,0,0,1,0, '');
DELETE FROM `waypoint_data` WHERE `id`=@PATH;
INSERT INTO `waypoint_data` (`id`,`point`,`position_x`,`position_y`,`position_z`,`orientation`,`delay`,`move_flag`,`action`,`action_chance`,`wpguid`) VALUES
(@PATH,1,-1928.789,-12869.43,86.3521,0,0,0,0,100,0), -- 09:15:24
(@PATH,2,-1926.808,-12868.42,86.50182,0,0,0,0,100,0), -- 09:15:32
(@PATH,3,-1915.362,-12863.29,87.48274,0,0,0,0,100,0), -- 09:15:37
(@PATH,4,-1895.262,-12862.47,87.17763,0,0,0,0,100,0), -- 09:15:44
(@PATH,5,-1888.151,-12870.06,85.72816,0,0,0,0,100,0), -- 09:15:50
(@PATH,6,-1881.628,-12885.19,81.05202,0,0,0,0,100,0), -- 09:15:58
(@PATH,7,-1882.931,-12895.07,76.1616,0,0,0,0,100,0), -- 09:16:03
(@PATH,8,-1896.266,-12915.07,69.35283,0,0,0,0,100,0), -- 09:16:13
(@PATH,9,-1908.279,-12924.68,66.72368,0,0,0,0,100,0), -- 09:16:19
(@PATH,10,-1933.388,-12933.09,64.4604,0,0,0,0,100,0), -- 09:16:28
(@PATH,11,-1947.921,-12933.25,61.58389,0,0,0,0,100,0), -- 09:16:36
(@PATH,12,-1964.046,-12931.37,54.13368,0,0,0,0,100,0), -- 09:16:45
(@PATH,13,-1985.971,-12921.06,46.1943,0,0,0,0,100,0), -- 09:16:54
(@PATH,14,-2008.113,-12899.47,43.04699,0,0,0,0,100,0), -- 09:17:06
(@PATH,15,-2009.894,-12861.36,40.86951,0,0,0,0,100,0), -- 09:17:15
(@PATH,16,-2002.674,-12851.02,38.75832,0,0,0,0,100,0), -- 09:17:26
(@PATH,17,-1982.862,-12829.53,35.24951,0,0,0,0,100,0), -- 09:17:38
(@PATH,18,-1963.942,-12804.68,29.6003,0,0,0,0,100,0), -- 09:17:50
(@PATH,19,-1947.879,-12792.53,25.92624,0,0,0,0,100,0), -- 09:18:00
(@PATH,20,-1936.599,-12789.01,24.07749,0,0,0,0,100,0), -- 09:18:06
(@PATH,21,-1922.057,-12789.15,21.28288,0,0,0,0,100,0), -- 09:18:15
(@PATH,22,-1919.057,-12789.16,20.79777,0,0,0,0,100,0), -- 09:18:16
(@PATH,23,-1910.856,-12787.78,18.40594,0,0,0,0,100,0), -- 09:18:17
(@PATH,24,-1865.516,-12783.02,8.171058,0,0,0,0,100,0), -- 09:18:33
(@PATH,25,-1828.088,-12780.68,5.281821,0,0,0,0,100,0), -- 09:18:47
(@PATH,26,-1821.925,-12778.83,4.372193,0,0,0,0,100,0), -- 09:18:55
(@PATH,27,-1808.465,-12775.22,3.311753,0,0,0,0,100,0), -- 09:19:11
(@PATH,28,-1840.72,-12785.45,6.14157,0,0,0,0,100,0), -- 09:19:26
(@PATH,29,-1857.858,-12783.72,7.100934,0,0,0,0,100,0), -- 09:19:32
(@PATH,30,-1882.401,-12782.23,10.96099,0,0,0,0,100,0), -- 09:19:47
(@PATH,31,-1918.945,-12788.79,20.91866,0,0,0,0,100,0), -- 09:20:03
(@PATH,32,-1946.804,-12792.22,25.9647,0,0,0,0,100,0), -- 09:20:12
(@PATH,33,-1957.95,-12798.83,28.51275,0,0,0,0,100,0), -- 09:20:18
(@PATH,34,-1972.832,-12814.32,31.96672,0,0,0,0,100,0), -- 09:20:28
(@PATH,35,-1998.71,-12847.28,38.17815,0,0,0,0,100,0), -- 09:20:41
(@PATH,36,-2010.876,-12865.39,41.37553,0,0,0,0,100,0), -- 09:20:52
(@PATH,37,-2008.604,-12898.32,42.78571,0,0,0,0,100,0), -- 09:21:03
(@PATH,38,-1999.134,-12909.23,44.42824,0,0,0,0,100,0), -- 09:21:12
(@PATH,39,-1977.069,-12925.9,48.62286,0,0,0,0,100,0), -- 09:21:25
(@PATH,40,-1960.811,-12931.79,56.09935,0,0,0,0,100,0), -- 09:21:34
(@PATH,41,-1938.755,-12933.6,63.95058,0,0,0,0,100,0), -- 09:21:43
(@PATH,42,-1901.555,-12919.94,68.1657,0,0,0,0,100,0), -- 09:21:59
(@PATH,43,-1892.329,-12910.31,70.69868,0,0,0,0,100,0), -- 09:22:05
(@PATH,44,-1881.857,-12893.26,77.24239,0,0,0,0,100,0), -- 09:22:15
(@PATH,45,-1882.775,-12878.57,83.94725,0,0,0,0,100,0), -- 09:22:21
(@PATH,46,-1903.748,-12860.37,87.55596,0,0,0,0,100,0), -- 09:22:30
(@PATH,47,-1921.45,-12865.49,86.99675,0,0,0,0,100,0), -- 09:22:34
(@PATH,48,-1928.858,-12869.55,86.33754,0,0,0,0,100,0); -- 09:22:43
-- 0x1C09E44240114C4000002500002C696C .go -1928.789 -12869.43 86.3521

-- Pathing for Bloodcursed Naga Entry: 17713 'TDB FORMAT' 
SET @NPC := 63492;
SET @PATH := @NPC * 10;
UPDATE `creature` SET `spawndist`=0,`MovementType`=2,`position_x`=-1796.121,`position_y`=-12771.99,`position_z`=2.181608 WHERE `guid`=@NPC;
DELETE FROM `creature_addon` WHERE `guid`=@NPC;
INSERT INTO `creature_addon` (`guid`,`path_id`,`mount`,`bytes1`,`bytes2`,`emote`,`auras`) VALUES (@NPC,@PATH,0,0,1,0, '');
DELETE FROM `waypoint_data` WHERE `id`=@PATH;
INSERT INTO `waypoint_data` (`id`,`point`,`position_x`,`position_y`,`position_z`,`orientation`,`delay`,`move_flag`,`action`,`action_chance`,`wpguid`) VALUES
(@PATH,1,-1796.121,-12771.99,2.181608,0,0,0,0,100,0), -- 09:11:36
(@PATH,2,-1808.465,-12775.22,3.311753,0,0,0,0,100,0), -- 09:11:53
(@PATH,3,-1840.747,-12785.32,6.340456,0,0,0,0,100,0), -- 09:12:08
(@PATH,4,-1857.898,-12783.8,7.071639,0,0,0,0,100,0), -- 09:12:14
(@PATH,5,-1882.401,-12782.29,10.8823,0,0,0,0,100,0), -- 09:12:29
(@PATH,6,-1921.118,-12788.98,21.68076,0,0,0,0,100,0), -- 09:12:45
(@PATH,7,-1946.973,-12792.21,25.87914,0,0,0,0,100,0), -- 09:12:55
(@PATH,8,-1957.784,-12798.69,28.39876,0,0,0,0,100,0), -- 09:13:01
(@PATH,9,-1972.893,-12814.4,31.85242,0,0,0,0,100,0), -- 09:13:10
(@PATH,10,-1998.871,-12847.28,38.12544,0,0,0,0,100,0), -- 09:13:23
(@PATH,11,-2009.722,-12860.75,40.77156,0,0,0,0,100,0), -- 09:13:34
(@PATH,12,-2008.599,-12898.46,42.77578,0,0,0,0,100,0), -- 09:13:46
(@PATH,13,-1998.956,-12909.28,44.34562,0,0,0,0,100,0), -- 09:13:55
(@PATH,14,-1977.231,-12926.03,48.51125,0,0,0,0,100,0), -- 09:14:07
(@PATH,15,-1960.969,-12931.87,56.10593,0,0,0,0,100,0), -- 09:14:15
(@PATH,16,-1938.779,-12933.54,63.8186,0,0,0,0,100,0), -- 09:14:25
(@PATH,17,-1913.007,-12927.43,66.26051,0,0,0,0,100,0), -- 09:14:34
(@PATH,18,-1898.97,-12917.32,68.56851,0,0,0,0,100,0), -- 09:14:42
(@PATH,19,-1893.777,-12912.74,70.14345,0,0,0,0,100,0), -- 09:14:47
(@PATH,20,-1881.76,-12893.25,77.15878,0,0,0,0,100,0), -- 09:14:57
(@PATH,21,-1882.529,-12878.56,83.82611,0,0,0,0,100,0), -- 09:15:03
(@PATH,22,-1903.836,-12860.37,87.51022,0,0,0,0,100,0), -- 09:15:11
(@PATH,23,-1921.361,-12865.68,87.0482,0,0,0,0,100,0), -- 09:15:17
(@PATH,24,-1928.789,-12869.43,86.3521,0,0,0,0,100,0), -- 09:15:24
(@PATH,25,-1926.808,-12868.42,86.50182,0,0,0,0,100,0), -- 09:15:32
(@PATH,26,-1915.362,-12863.29,87.48274,0,0,0,0,100,0), -- 09:15:37
(@PATH,27,-1895.262,-12862.47,87.17763,0,0,0,0,100,0), -- 09:15:44
(@PATH,28,-1888.151,-12870.06,85.72816,0,0,0,0,100,0), -- 09:15:50
(@PATH,29,-1881.628,-12885.19,81.05202,0,0,0,0,100,0), -- 09:15:58
(@PATH,30,-1882.931,-12895.07,76.1616,0,0,0,0,100,0), -- 09:16:03
(@PATH,31,-1896.266,-12915.07,69.35283,0,0,0,0,100,0), -- 09:16:13
(@PATH,32,-1908.279,-12924.68,66.72368,0,0,0,0,100,0), -- 09:16:19
(@PATH,33,-1933.388,-12933.09,64.4604,0,0,0,0,100,0), -- 09:16:28
(@PATH,34,-1947.921,-12933.25,61.58389,0,0,0,0,100,0), -- 09:16:36
(@PATH,35,-1964.046,-12931.37,54.13368,0,0,0,0,100,0), -- 09:16:45
(@PATH,36,-1985.971,-12921.06,46.1943,0,0,0,0,100,0), -- 09:16:54
(@PATH,37,-2008.113,-12899.47,43.04699,0,0,0,0,100,0), -- 09:17:06
(@PATH,38,-2009.894,-12861.36,40.86951,0,0,0,0,100,0), -- 09:17:15
(@PATH,39,-2002.674,-12851.02,38.75832,0,0,0,0,100,0), -- 09:17:26
(@PATH,40,-1982.862,-12829.53,35.24951,0,0,0,0,100,0), -- 09:17:38
(@PATH,41,-1963.942,-12804.68,29.6003,0,0,0,0,100,0), -- 09:17:50
(@PATH,42,-1947.879,-12792.53,25.92624,0,0,0,0,100,0), -- 09:18:00
(@PATH,43,-1936.599,-12789.01,24.07749,0,0,0,0,100,0), -- 09:18:06
(@PATH,44,-1922.057,-12789.15,21.28288,0,0,0,0,100,0), -- 09:18:15
(@PATH,45,-1919.057,-12789.16,20.79777,0,0,0,0,100,0), -- 09:18:16
(@PATH,46,-1910.856,-12787.78,18.40594,0,0,0,0,100,0), -- 09:18:17
(@PATH,47,-1870.516,-12782.52,8.421058,0,0,0,0,100,0), -- 09:18:33
(@PATH,48,-1828.088,-12780.68,5.281821,0,0,0,0,100,0), -- 09:18:47
(@PATH,49,-1821.925,-12778.83,4.372193,0,0,0,0,100,0), -- 09:18:55
(@PATH,50,-1808.465,-12775.22,3.311753,0,0,0,0,100,0); -- 09:19:11
-- 0x1C09E44240114C4000002500002C696C .go -1796.121 -12771.99 2.181608

-- Bloodcursed Naga SAI
SET @ENTRY := 17713;
UPDATE `creature_template` SET `AIName`="SmartAI" WHERE `entry`=@ENTRY;
DELETE FROM `smart_scripts` WHERE `entryorguid`=@ENTRY AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(@ENTRY,0,0,0,25,0,100,0,0,0,0,0,11,12544,0,0,0,0,0,1,0,0,0,0,0,0,0,"Bloodcursed Naga - On Reset - Cast 'Frost Armor'");

-- Pathing for Lord Xiz Entry: 17701 'TDB FORMAT' 
SET @NPC := 63448;
SET @PATH := @NPC * 10;
UPDATE `creature` SET `spawndist`=0,`MovementType`=2,`position_x`=-2226.604,`position_y`=-12321.21,`position_z`=57.54912 WHERE `guid`=@NPC;
DELETE FROM `creature_addon` WHERE `guid`=@NPC;
INSERT INTO `creature_addon` (`guid`,`path_id`,`mount`,`bytes1`,`bytes2`,`emote`,`auras`) VALUES (@NPC,@PATH,0,0,1,0, '');
DELETE FROM `waypoint_data` WHERE `id`=@PATH;
INSERT INTO `waypoint_data` (`id`,`point`,`position_x`,`position_y`,`position_z`,`orientation`,`delay`,`move_flag`,`action`,`action_chance`,`wpguid`) VALUES
(@PATH,1,-2253.258,-12320.91,57.29495,0,0,0,0,100,0), -- 09:08:14
(@PATH,2,-2214.95,-12321.52,57.30329,0,0,0,0,100,0); -- 09:08:29
-- 0x1C09E4424011494000002500002C576E .go -2226.604 -12321.21 57.54912