#ifndef _MFLEET_H
#define _MFLEET_H

#include "../melee.h"

class Fleet;
extern Fleet *reference_fleet;


#include <vector> //needed for STL list, the using namespace thingy allows STL classes
using namespace std;


//TODO get rid of global variables used by this function; remove this function
void init_fleet();	// inits reference_fleet [former shiptype array]


/**	\brief	Contains a list of ships.
*/
class Fleet{


protected:
    typedef unsigned int Index;
    typedef ShipType * MyFleetShipType;
    typedef vector<MyFleetShipType> MyFleetListType;

public:
    //the maxiumum length of the fleet title
    enum { MAX_TITLE_LENGTH = 80 };

    //dunno why this doesn't work in VC++ ... ?
    //const static int MAX_TITLE_LENGTH = 80;

    enum SortingMethod { 
        SORTING_METHOD_DEFAULT = 0, /**< Currently does the same thing as SORTING_METHOD_NAME_DESCENDING */
        SORTING_METHOD_NAME_DESCENDING, /**< combined Specied/ship name */
        SORTING_METHOD_NAME_ASCENDING, 
        SORTING_METHOD_COST_DESCENDING,
        SORTING_METHOD_COST_ASCENDING, 
        SORTING_METHOD_TWCOST_ASCENDING, 
        SORTING_METHOD_TWCOST_DESCENDING,
        SORTING_METHOD_NAME1_ASCENDING, /**< Species name */
        SORTING_METHOD_NAME1_DESCENDING, /**< Species name */
        SORTING_METHOD_NAME2_ASCENDING, /**< Ship name (not including species) */
        SORTING_METHOD_NAME2_DESCENDING, /**< Ship name (not including species) */
        SORTING_METHOD_CODERS_ASCENDING, /**<  */
        SORTING_METHOD_CODERS_DESCENDING, /**<  */
        SORTING_METHOD_ORIGIN_ASCENDING, /**< SC1-3, TW, or the group that made the ship */
        SORTING_METHOD_ORIGIN_DESCENDING /**< SC1-3, TW, or the group that made the ship */
    };




	/** \brief Default constructor with zero ships */
    Fleet();

	
    /** \brief removes all ships and sets the cost to zero */
    void reset();

	
    /** \brief dumps out the fleet out to a buffer
        \param psize the size of the buffer
        \return a pointer to the buffer */
    void *serialize (int *psize);

    /** \brief reads in the fleet from a specified buffer
        \param data the buffer to read from
        \param psize the size of the buffer*/
	void deserialize(void *data, int psize);

	/** \brief adds a ship to this fleet
        \param type the type of ship to add
        \return the slot the ship type was added to
    */
    int addShipType( ShipType * type );


    //depricated (no longer in use)
    //void select_slot(int slot, ShipType *type);

	
    
    /** \brief removes a given ship from the specified ship slot
        \param slot the slot the ship to be removed appears in */
    void clear_slot (int slot);

    /** \brief saves the fleet to a config file, given the config filename and section
        \param filename the filename to open and save to
        \param section the section within the specified config file to save to
        \TODO report an error code
    */
    void save(const char *filename, const char *section);

    /** \brief loads the fleet from a config file (to this fleet), given the filename and section of a config file.
        \param filename the filename to save to
        \section the section within the config file to save to
    */
	void load(const char *filename, const char *section);

	/** \brief sorts this fleet according to each ShipType's operator<= function*/
    //void sort();

	

	/** \brief sorts this fleet according to the given compare function.  May be called as Sort();
        \param method The method with which to sort.  Options are:
            -SORTING_METHOD_DEFAULT,
            -SORTING_METHOD_NAME_DESCENDING,
            -SORTING_METHOD_NAME_ASCENDING,
            -SORTING_METHOD_COST_DESCENDING,
            -SORTING_METHOD_COST_ASCENDING,
            -SORTING_METHOD_TWCOST_ASCENDING,
            -SORTING_METHOD_TWCOST_DESCENDING,
            -SORTING_METHOD_NAME1_ASCENDING,
            -SORTING_METHOD_NAME1_DESCENDING,
            -SORTING_METHOD_NAME2_ASCENDING,
            -SORTING_METHOD_NAME2_DESCENDING,
            -SORTING_METHOD_CODERS_ASCENDING,
            -SORTING_METHOD_CODERS_DESCENDING,
            -SORTING_METHOD_ORIGIN_ASCENDING,
            -SORTING_METHOD_ORIGIN_DESCENDING
        \param startIndex (default 0) the index of the first ship to sort
        \param endIndex (default -1) the index of the last ship in the fleet to sort.  -1 means the last ship.
    */
    void Sort(SortingMethod method=SORTING_METHOD_DEFAULT, int startIndex=0, int endIndex=-1);

    
    /** \brief returns the number of ships in this fleet
        \return the number of ships in this fleet*/
    int inline getSize() { return ships.size(); }

    inline int getCost() { return cost; } 
    inline int setCost(int newCost) { cost = newCost; return cost; }

    inline char * getTitle() { return title; }
    inline char * setTitle(char * newTitle) { strncpy(title, newTitle, MAX_TITLE_LENGTH); return title; }

    /** \brief adds the ships in the specified fleet to this one
        \param fleetToAdd the fleet with ships to add */
    void addFleet(Fleet * fleetToAdd);



    /** \brief Returns the ship in a particular slot.
        \param slot The slot of the ShipType to return.  Returns NULL if slot is not (0 <= slot <= getSize())
        \return The chosen ShipType, or NULL if slot was outside the selectable range of ShipTypes.*/    
    ShipType * getShipType( int slot );

    /** \brief Returns the ship in a particular slot.
        \param offset The slot of the ShipType to return.  Returns NULL if slot is not (0 <= slot <= getSize())
        \return The chosen ShipType, or NULL if slot was outside the selectable range of ShipTypes.*/    
    ShipType * operator[](int offset) {
        return getShipType( offset );
    }



protected:
    //STL list
    
	/** \brief the title of this fleet*/
    char title[MAX_TITLE_LENGTH];

    /** \brief the total cost of all ships in this fleet */
	int cost;

    /** \brief the list of ships*/
    MyFleetListType ships;


};






#endif

