General info: 

tools that keep track and actually pass information between them to help players as well as add a bit more flexibility to the game (as i understand it). it would use an arduino board (dual core with wifi for updating and such if needed), an sd card to move character sheets around, lcd or touch screen for selecting items or spells or specific attacks, gps for exact coordinates, 6 axis accelerometer for swings and direction (includes a compass), buttons for selections on the screen, and a radio system for actual communication (maybe a speaker for sounds and such for stuff like "freeze" or "help" or such...hell could be used by an npc).  the spells and attacks could possibly (thinking out loud per say)  be linked to a specific movement of the device (downward slash of a sword vs stabbing it into the ground or a twist of the wand vs a flick of the wand). the gps  and compass are both due to the distance and direction factors of attacks and range of radio. if some one is attacking with a direct attack but there is another player down range. their device would receive the info on the spell (range, damage, effect)  and the coords of the attacker and ignore it (or do what the effect of the spell is from that range. the way to prevent people from just switching out sd cards would have to be a handshake of all the devices done either at the beginning or in the file of the sd card (a little string that is sent with the msgs to verify the spell or effect). 

The best part of this system is that you could add a random device on the ground that is an npc or dragon that they could interact with (attack, help, etc). you can incorporate raids this way or add quests and loot. no idea how to do the item system yet though but first things first as it can be added later when the basics are up and running.


More Specific info and testing device speficications:

this program will use arduino to add to the LARPing (Live Action Role Playing) realism as well as add to the flexibility of the DM (Dungeon Masters) ability to add content (dragons or other npcs).


The arduino is an ESP32 muticore (https://www.amazon.com/gp/product/B01N0SB08Q/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)this will allow the device to respond to input while also being able to do its job of keeping track of everything
the sd card and all data will be saved in a microsd card. the module for reading it during testing is a HiLetGo SPI intereface module (https://www.amazon.com/gp/product/B07BJ2P6X6/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)
the GPS module is a NEO-6M (https://www.amazon.com/Microcontroller-Compatible-Sensitivity-Navigation-Positioning/dp/B07P8YMVNT/ref=sr_1_3?crid=LZI0I85I6OTG&keywords=gps+arduino&qid=1561183900&s=gateway&sprefix=gps+ard%2Caps%2C492&sr=8-3)
the LCD is an OLED i2c white black screen (https://www.amazon.com/gp/product/B01HHOAQ5A/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&th=1)
the accelerometer is a 9 axis sensor with a built in compass (3 axis gyro, 3 axis accelerometer, 3 axis magnetometer) with i2c communication (https://www.amazon.com/gp/product/B01DIGRR8U/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)
the radio uses a 2.4-2.5 GHz band and uses a spi connection. has a range of 80 meters in ideal conditions (which is way more then enough even in non ideal conditions like the forest or in buildings) only issue is it cannot recieve while sending data, so read data until it must be sent then send in a burst. also is a tiny bit directional (https://www.amazon.com/gp/product/B072BLN8SZ/ref=ppx_yo_dt_b_asin_title_o03_s00?ie=UTF8&psc=1)
to do sound would require a speaker as well as a mp3 player module due to the sound modulator and such (https://www.amazon.com/gp/product/B06WP9RQ9V/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)



now to ideas and thoughts: 

use a contral device to input player info and put it into each device. this will prevent tampering and secondary devices being added. also add verification code here so that no one can just add a device
posibly use wifi to do the transfer with a confirmation screen (pi?)
find way to save move activations on the divice via doing the movement over time



moves would have to have: 
	Player name
		for recording who cast it
	Spell name
		this is so it can be recorded into a log to look back at later
	GPS	
		this would tell the device where the player is in relation to it
	heading
		direction spell is cast
	cast angle
		this is a theoretical idea and may not be worth it
		where is the spell being cast? is it being cast at the ground and not effecting anyone? at the sky? 
	number of effects
		as there may be more then one effect		
			fireball has direct damage as well as possibility of burn which is over time damage
	radius of effect 
		start and end zone as there may be spells that require a zone to be selected for a heal. 
		it would have to be radius as we are talking about cones of effect and that implies circles.
		this would also allow a player to select a ring around them where the center is free of an effect (firewall) and the area around that would be on fire, hurting all that pass it)
		this would require that the device constantly sends out a pulse of what is going on with the effect. 
			add update token to the message that informs that the effect is being updated constantly so its not stacked on accident?
	cone of effect
		distance around the player (x degrees would be 1/2 x degrees from the direction the player is pointing on each side. 360 would be 180 around on each side or all the way around)
	actual effect
		this would allow spells that can cause secondary effect like burn where it does damage over time.
	duration 
		how long spell lasts or how long its effect lasts
	cast time 
		how long the device must be held to cast 
			casting a massive aoe heal spell would take longer then a fireball or bolt and a player hitting you may inturrupt the spell
	mana cost
		how expensive is it to cast 
			
			
possible radio message (this will change as its just off what is above right now):

"[code][playerName][Spell Name][GPS][Heading][cast angle][Number Of Effects]{[Effect one][Radius Start][Radius End][Degree of effect][Duration]}{[Effect two][Radius Start][Radius End][Degree of effect][Duration]}"
	
this would be taken and broken into its constituant parts based upon where everything is. the number of effects would be used to run a while or for loop to get the effects.
if any of the effect(s) hit the player, then that effect, the spell name and player name would be saved to a log with the before and after health. if its an effect that is over time
then the log will reflect it ("x's burn hits y for z damage 120->110" or such)
anything that modifies a player value (health, mana or such (is there anything else?)) should be recorded in a log.
