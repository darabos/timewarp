-- Default dialog screen


function DIALOG ()
DialogStart "gamedata/dialogs/arilou.jpg"
	DialogSetMusic "gamedata/dialogs/arilou.mod"
	DialogWrite "Hi!"
	answer = DialogAnswer ( "Bye" )
DialogEnd()

end