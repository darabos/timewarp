#ifndef _MFLEET_H
#define _MFLEET_H

#include "../melee.h"

class Fleet;
extern Fleet *reference_fleet;


#include <list> //needed for STL list, the using namespace thingy allows STL classes
using namespace std;


//TODO get rid of global variables used by this function; remove this function
void init_fleet();	// inits reference_fleet [former shiptype array]


/**	\brief	Contains a list of ships.
*/
class Fleet{


protected:
    typedef unsigned int Index;
    typedef ShipType * MyFleetShipType;
    typedef list<MyFleetShipType> MyFleetListType;

public:
    //the maxiumum length of the fleet title
    enum { MAX_TITLE_LENGTH = 80 };    
    //const static int MAX_TITLE_LENGTH = 80;


	//default constructor with zero ships
    Fleet();

	
    //removes all ships and sets the cost to zero
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

    
    //deprecated
    //	int ship_idtoindex(const char *id_str);

	
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
    void sort();

	

	/** \brief sorts this fleet according to the given compare function 
        \param compare_function the function to compare ShipTypes with
        \TODO reimplement this function; make each sort type available from a public context
    */
    void sort ( int (*compare_function)(const Index *, const Index *)) ;

    /**	\brief  function to compare ShipTypes with, such that they will be sorted alphabetically
        \param	the item to compare ...?	
        \param	??? */
	void sort_alphabetical(const char *item, const char *section = "Info") ;


    /**	\brief  function to compare ShipTypes with, such that they will be sorted numerically
        \param	the item to compare ...?	
        \param	??? */
	void sort_numerical(const char *item, const char *section = "Info") ;


    /**	\brief  function to compare ShipTypes with, such that they will be sorted by their names
        \param	the item to compare ...?	
        \param	??? */
    int fleetsort_by_name ( const Index *_i1, const Index *_i2 ) ;

    
    /** \brief returns the number of ships in this fleet
        \return the number of ships in this fleet*/
    int inline getSize() { return ships.size(); }

    inline int getCost() { return cost; } 
    inline int setCost(int newCost) { cost = newCost; return cost; }

    inline char * getTitle() { return title; }
    inline char * setTitle(char * newTitle) { strncpy(title, newTitle, MAX_TITLE_LENGTH); return title; }



    /** \brief Returns the ship in a particular slot.
        \param slot The slot of the ShipType to return.  Returns NULL if slot is not (0 <= slot <= getSize())
        \return The chosen ShipType, or NULL if slot was outside the selectable range of ShipTypes.*/    
    ShipType * getShipType( int slot );

    /** \brief Returns the ship in a particular slot.
        \param offset The slot of the ShipType to return.  Returns NULL if slot is not (0 <= slot <= getSize())
        \return The chosen ShipType, or NULL if slot was outside the selectable range of ShipTypes.*/    
/*    ShipType * operator[](int offset) {
        return getShipType( offset );
    }*/



protected:
    //STL list
    
	//the title of this fleet
    char title[MAX_TITLE_LENGTH];

    //the total cost of all ships in this fleet
	int cost;

    //the list of ships
    MyFleetListType ships;

};


/*int fleetsort_clean ( const Fleet::Index *_i1, const Fleet::Index *_i2 ) ;
int fleetsort_by_name ( const Fleet::Index *_i1, const Fleet::Index *_i2 ) ;
int fleetsort_by_cost ( const Fleet::Index *_i1, const Fleet::Index *_i2 ) ;*/

#endif

