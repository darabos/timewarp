----------------------------------------------------
--a sample universe
----------------------------------------------------
require ("StarClasses")


if (sampleUniverse == nil) then

  sampleUniverse = Universe:new {
         sol = Star:new{ name="Sol", x=50, y=30 },
         earth = Planet:new{ name="Earth", x=55, y=35, minerals=50 },
         luna = Planet:new{ name="Luna", x=57, y=32 },
         ACentari = Star:new{ name="Alpha Centari", x=200,y=10 },
  }
end






