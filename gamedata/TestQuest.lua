Complited           = 0; -- Quest complitted
shoscKilled         = 1; -- Shofixti Scout you need to kill
allready_get_reward = 0;

function Process(time, enemy, kills, x, y, res_num1, res_num2, res_str)
end

function GAME_EVENT_SHIP_DIE( Type )
	if Type == "shosc" then shoscKilled = shoscKilled - 1 end
	if shoscLilled == 0 then Complited = 1 end
end

function GAME_EVENT_VISIT_LOCATION( locationID )
	if locationID == QuestSourceLocationID then
		if Complited == 1 then return Reward() end
		return HurryLazyPlayer()
	end
end

function Reward()
DialogStart "gamedata/pkunk-standing.bmp"
	DialogWrite "Congratulations, those bastards are all dead now! Take 20 bucks"
	answer = DialogAnswer ( "Thanks!" )
DialogEnd()
end

function HurryLazyPlayer()
DialogStart "gamedata/pkunk-standing.bmp"
	DialogWrite "Hurry, they are criminals!"
	answer = DialogAnswer ( "I am off!!!" )
DialogEnd()
end


DialogStart "gamedata/pkunk-standing.bmp"
function question1()
	DialogWrite "Hello, my spiritual child. I have no quest for you yet"
	answer = DialogAnswer("But I am realy need one!", 
			"Lets just talk a bit!!!",
			 "bye")
	if answer == 1 then return question2() end
	if answer == 2 then return question3() end
	if answer == 3 then return DialogWrite "Bye" end
end

function question2()
	DialogWrite "Sorry, this part of game is on very early development stage. But don't lose hope :)"
	answer = DialogAnswer("Ok, lets just talk then", "Bye")
	if answer == 1 then return question3() end
	if answer == 2 then return DialogWrite "Bye" end 
end

function question3()
	DialogWrite "And what are we doing now?"
	answer = DialogAnswer ("Testing!!!", "Bye")
	if answer == 1 then return DialogWrite ("Booring..."); end
	if answer == 2 then return question1() end
end

question1()

DialogEnd()

