Complited           = 0; -- Quest complitted
shoscKilled         = 1; -- Shofixti Scout you need to kill
allready_get_reward = 0;

function Process(time, enemy, kills, x, y, res_num1, res_num2, res_str)
end

function GAME_EVENT_SHIP_DIE( Type )
	if Type == "shosc" then shoscKilled = shoscKilled - 1 end
	if shoscKilled == 0 then Complited = 1 end
end

function GAME_EVENT_ENTER_STATION( locationID )
	if allready_get_reward == 1 then return end
--	if locationID == QuestSourceLocationID then
		if Complited == 1 then return Reward() end
		return HurryLazyPlayer()
--	end
end

function Reward()
DialogStart "gamedata/pkunk-standing.bmp"
	DialogWrite "Congratulations, those bastards are all dead now! Take 20 bucks"
	answer = DialogAnswer ( "Thanks!" )
	AddBuckazoids(20);
	allready_get_reward = 1;
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
	DialogWrite "Hello, my spiritual child. I kill Shofixti Scout for me."
	answer = DialogAnswer("Why?", 
			      "I agree")
	if answer == 1 then return question2() end
	if answer == 2 then return question3() end
end

function question2()
	DialogWrite "They influence badly on our Ye-hat brothers"
	answer = DialogAnswer("I agree")
	if answer == 1 then return question3() end
end

function question3()
	DialogWrite "What are you waiting for?"
	answer = DialogAnswer ("Bye")
	if answer == 1 then return DialogWrite ("Bye"); end
end

question1()

DialogEnd()

