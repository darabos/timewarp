-- Raw Dialog to learn things 

SetAlienImage "pkunk-standing.bmp"

function question1()
	Write "Ye-hat brother! Do you wand to hear about spirital potworm?"
	answer = Answer("Begone, ugly bird!!!", "NRRRRRRR!!!", "bye")
	if answer == 1 then return Fight() end
	if answer == 2 then return question2() end
	if answer == 3 then return Write "Bye" end
end

function question2()
	Write "I am not going to speak with such brute as you"
	answer = Answer("Bye", "Lets start over")
	if answer == 1 then return Write "Bye" end
	if answer == 2 then Write "K\n" return question1() end 
end

question1()
