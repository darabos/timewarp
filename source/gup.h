/*
This is the .h file for Gobs upgrades

They are kinda hardwired for working with Gob, but you might 
be able to get most of them to work for some other game type
*/

class Upgrade;

extern Upgrade **upgrade_list;

namespace UpgradeIndex {
	enum {
		crewpod,
		battery,
		thrusters,
		controljets,
		dynamo,
		supoxrange,
		supoxdamage,
		supoxblade,
		orzmissile,
		orzmarinespeed,
		orzabsorption,
		kohrahbladedamage,
		kohrahbladespeed,
		kohrahfirerange,
		kohrahfiredamage,
		divinefavor,
		defender,
		planetlocater,
		hyperdynamo
	};
};