--
-- this is a bridge between the Lua and C++ worlds.  Very experimental-like!
--

nast = 0;

function calculate()

	if nast < 30 then
		addAsteroid();
		nast = nast + 1
	end
end


-- JUST THE INITIALIZATION...

--require "StarClasses"
--require "SampleUniverse"

addAsteroid();
addAsteroid();
addAsteroid();

addSpecialAsteroid("Wakko");
addSpecialAsteroid("Yakko");
addSpecialAsteroid("Dot");



addPlanet(100,100);

--addPlanet(150,150);
--addPlanet(200,200);
--addPlanet(250,250);















