-------------------------------------------------------------------------------
--StarClasses.lua   Defined a bunch of classes which describe the universe
-------------------------------------------------------------------------------

require("Class")

SpaceType = { TrueSpace="TrueSpace",
              HyperSpace="HyperSpace",
              QuasiSpace="QuasiSpace",
              HeavySpace="HeavySpace" }

SpaceTypeMessage = { TrueSpace="Everything looks normal.",
                     HyperSpace="Everything looks red and strange.",
                     QuasiSpace="Everything is green",
                     HeavySpace="You can't see anything.  You are likely to be *eaten* by an Orz." }


-------------------------------------------------------------------------------
-- SpaceObject Class
-------------------------------------------------------------------------------
SpaceObject = Class(nil)

SpaceObject.x = 0
SpaceObject.y = 0
SpaceObject.name = ""

function SpaceObject:playerCollide()
  print("Player Collided with this space object")
  print("x==",self.x)
end

-------------------------------------------------------------------------------
-- Star Class
-------------------------------------------------------------------------------
Star = Class(SpaceObject)
Star.pictureFilename = ""

function Star:playerCollide(player)
  print("\""..player.name.."\" collided with \""..self.name.."\"")
  player.spaceType = SpaceType.TrueSpace
end


-------------------------------------------------------------------------------
-- Planet Class
-------------------------------------------------------------------------------
Planet = Class(SpaceObject)

function Planet:playerCollide()
  print("Player Collided with this Planet")
end


Techtonics = { lowest=0, medium=64, extreme=128, highest=128, default=8 }
--Planet.techtonics
Planet.techtonics = Techtonics.default

Minerals = { lowest=0, medium=64, extreme=128, highest=128, default = 8 }

--Planet.minerals
Planet.minerals = Minerals.default

MineralType = { radioactive="Radioactive", metal="Metal", acid="Acid", preciousMetal="Precious Metals" }
MineralType.default = MineralType.metal

--Planet.mineralType
Planet.mineralType = MineralType.default

WeatherType = { lowest=0, medium=64, extreme=128, highest=128, default = 8 }
Planet.weather = WeatherType.default


-------------------------------------------------------------------------------
-- Player Class
-------------------------------------------------------------------------------
Player = Class(nil)

Player.name = "Player Name"
Player.spaceType = SpaceType.HyperSpace
Player.HyperSpaceX = 100
Player.HyperSpaceY = 100
Player.TrueSpaceX = 0
Player.TrueSpaceY = 0
Player.QuasiSpaceX = 0
Player.QuasiSpaceY = 0
Player.HeavySpaceX = 0
Player.HeavySpaceY = 0
Player.spaceType = SpaceType.HyperSpace

function Player:getPosition()
    return "("..self[self.spaceType.."X"].. ",".. self[self.spaceType.."Y"]
          .. ") in " .. SpaceType[self.spaceType]
end

--function Player:setPosition(x, y, _SpaceType)
--   self.["spaceType"..X] .. "," .. self.["spaceType"..Y] .. ") in "
--         .. SpaceType[spaceType])
--end




-------------------------------------------------------------------------------
-- Universe Class
-------------------------------------------------------------------------------
Universe = Class(nil)

-- Universe.truespaceObjectList = { }
-- Universe.hyperspaceObjectList = { }
-- Universe.quasispaceObjectList = { }
-- Universe.heavyspaceObjectList = { }
-- 
-- function Universe:addStar(star)
--   assert(star, "Universe:addStar(): Can't add nil star")
--   hyperspaceObjectList[star.id] = star
-- end

function Universe:showLocalObjects( spaceType )
    assert(spaceType==SpaceType.TrueSpace or
           spaceType==SpaceType.HyperSpace or
           spaceType==SpaceType.QuasiSpace or
           spaceType==SpaceType.HeavySpace,
           "showLocalObjects(): unknown spaceType - ", spaceType)

    if (spaceType==SpaceType.TrueSpace) then
      print( "ID             Name             (x,y) ")
      for k,v in pairs(self) do
          if (v:isa(Planet)) then
             print(string.format("%8s %18s (%3s,%3s)", k, v.name, v.x, v.y ))
          elseif (v:isa(Star)) then
             print(string.format("%8s %18s (%3s,%3s)", k, v.name, v.x, v.y ))
          end
      end
    end

    if (spaceType==SpaceType.HyperSpace) then
      print( "ID             Name             (x,y) ")
      for k,v in pairs(self) do
          if (v:isa(Star)) then
             print(string.format("%8s %18s (%3s,%3s)", k, v.name, v.x, v.y ))
          end
      end
    end

    if (spaceType==SpaceType.QuasiSpace) then
      print("TODO")
    end

    if (spaceType==SpaceType.HeavySpace) then
      print("TODO")
    end

end

function Universe:showUniverse()
    print( "ID             Name   (x,y) Minerals(quantity) Minerals(type) Weather Techtonics")

    for k,v in pairs(self) do
        print(string.format("%8s %8s (%3s,%3s)",
          tostring(k), tostring(v.name), v.x, v.y ))
--        print(string.format("%8s %8s (%3s,%3s) %18s %14s %7s %10s",
--              tostring(k), tostring(v.name), v.x, v.y, v.minerals, v.mineralType, v.weather, v.techtonics ))
    end
end



function Universe:handleCollisions( player )
    assert(player.spaceType==SpaceType.TrueSpace or
           player.spaceType==SpaceType.HyperSpace or
           player.spaceType==SpaceType.QuasiSpace or
           player.spaceType==SpaceType.HeavySpace,
           "showLocalObjects(): unknown spaceType")

    if (player.spaceType==SpaceType.TrueSpace) then
      for k,v in pairs(self) do
          if (v:isa(Planet) and v.x==player.HyperSpaceX and v.y==player.HyperSpaceY) then
             v:playerCollide(player)
          elseif (v:isa(Star) and v.x==player.HyperSpaceX and v.y==player.HyperSpaceY) then
             v:playerCollide(player)
          end
      end
    end

    if (player.spaceType==SpaceType.HyperSpace) then
      for k,v in pairs(self) do
          if (v:isa(Star) and v.x==player.HyperSpaceX and v.y==player.HyperSpaceY) then
             v:playerCollide(player)
          end
      end
    end

    if (player.spaceType==SpaceType.QuasiSpace) then
      print("TODO")
    end

    if (player.spaceType==SpaceType.HeavySpace) then
      print("TODO")
    end

end

