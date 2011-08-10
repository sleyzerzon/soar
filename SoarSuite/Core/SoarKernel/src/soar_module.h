/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  soar_module.h
 *
 * =======================================================================
 */

#ifndef SOAR_MODULE_H
#define SOAR_MODULE_H

#include <portability.h>

#include <map>
#include <string>
#include <set>
#include <list>
#include <functional>
#include <assert.h>

#include "misc.h"
#include "symtab.h"
#include "mem.h"

typedef struct wme_struct wme;
typedef struct preference_struct preference;

// separates this functionality
// just for Soar modules
namespace soar_module
{
	/////////////////////////////////////////////////////////////
	// Utility functions
	/////////////////////////////////////////////////////////////

	typedef std::set< wme* > wme_set;
	
	typedef struct symbol_triple_struct
	{
		Symbol* id;
		Symbol* attr;
		Symbol* value;

		symbol_triple_struct( Symbol* new_id, Symbol* new_attr, Symbol* new_value ): id(new_id), attr(new_attr), value(new_value) {}
	} symbol_triple;
	typedef std::list< symbol_triple* > symbol_triple_list;
	
	wme *add_module_wme( agent *my_agent, Symbol *id, Symbol *attr, Symbol *value );
	void remove_module_wme( agent *my_agent, wme *w );
	instantiation* make_fake_instantiation( agent* my_agent, Symbol* state, wme_set* conditions, symbol_triple_list* actions );
	
	///////////////////////////////////////////////////////////////////////////
	// Predicates
	///////////////////////////////////////////////////////////////////////////
	
	// a functor for validating parameter values
	template <typename T>
	class predicate: public std::unary_function<T, bool>
	{
		public:
			virtual ~predicate() {}
			virtual bool operator() ( T /*val*/ ) { return true; }
	};	

	// a false predicate
	template <typename T>
	class f_predicate: public predicate<T>
	{
		public:			
			virtual bool operator() ( T /*val*/ ) { return false; }
	};

	// predefined predicate for validating
	// a value between two values known at
	// predicate initialization
	template <typename T>
	class btw_predicate: public predicate<T>
	{
		private:
			T my_min;
			T my_max;
			bool inclusive;
		
		public:
			btw_predicate( T new_min, T new_max, bool new_inclusive ): my_min( new_min ), my_max( new_max ), inclusive( new_inclusive ) {}

			bool operator() ( T val )
			{
				return ( ( inclusive )?( ( val >= my_min ) && ( val <= my_max ) ):( ( val > my_min ) && ( val < my_max ) ) );
			}
	};
	
	// predefined predicate for validating
	// a value greater than a value known at
	// predicate initialization
	template <typename T>
	class gt_predicate: public predicate<T>
	{
		private:
			T my_min;			
			bool inclusive;
		
		public:
			gt_predicate( T new_min, bool new_inclusive ): my_min( new_min ), inclusive( new_inclusive ) {}
			
			bool operator() ( T val )
			{
				return ( ( inclusive )?( ( val >= my_min ) ):( ( val > my_min ) ) );
			}
	};
		
	// predefined predicate for validating
	// a value less than a value known at
	// predicate initialization
	template <typename T>
	class lt_predicate: public predicate<T>
	{
		private:
			T my_max;			
			bool inclusive;
		
		public:
			lt_predicate( T new_max, bool new_inclusive ): my_max( new_max ), inclusive( new_inclusive ) {}		
			
			bool operator() ( T val )
			{
				return ( ( inclusive )?( ( val <= my_max ) ):( ( val < my_max ) ) );
			}
	};

	// superclass for predicates needing
	// agent state
	template <typename T>
	class agent_predicate: public predicate<T>
	{
		protected:
			agent *my_agent;

		public:
			agent_predicate( agent *new_agent ): my_agent( new_agent ) {}
	};


	///////////////////////////////////////////////////////////////////////////
	// Common for params, stats, timers, etc.
	///////////////////////////////////////////////////////////////////////////

	class named_object
	{
		private:
			const char *name;

		public:
			named_object( const char *new_name ): name( new_name ) {}
			virtual ~named_object() {}

			//

			const char *get_name()
			{
				return name;
			}

			//

			virtual char *get_string() = 0;
	};


	template <typename T>
	class accumulator: public std::unary_function<T, void>
	{
		public:
			virtual ~accumulator() {}

			virtual void operator() ( T /*val*/ ) {}
	};

		
	// this class provides for efficient 
	// string->object access
	template <class T>
	class object_container
	{					
		protected:
			agent *my_agent;
			std::map<std::string, T *> *objects;
			
			void add( T *new_object )
			{
				std::string temp_str( new_object->get_name() );
				(*objects)[ temp_str ] = new_object;
			}

		public:
			object_container( agent *new_agent ): my_agent( new_agent ), objects( new std::map<std::string, T *> ) {}
			
			virtual ~object_container()
			{
				typename std::map<std::string, T *>::iterator p;

				for ( p=objects->begin(); p!=objects->end(); p++ )
					delete p->second;

				delete objects;
			}

			//

			T *get( const char *name )
			{
				std::string temp_str( name );
				typename std::map<std::string, T *>::iterator p = objects->find( temp_str );

				if ( p == objects->end() )
					return NULL;
				else
					return p->second;
			}

			void for_each( accumulator<T *> &f  )
			{
				typename std::map<std::string, T *>::iterator p;

				for ( p=objects->begin(); p!=objects->end(); p++ )
				{
					f( p->second );
				}
			}
	};


	///////////////////////////////////////////////////////////////////////////
	// Parameters
	///////////////////////////////////////////////////////////////////////////
	
	// all parameters have a name and
	// can be manipulated generically
	// via strings
	class param: public named_object
	{			
		public:		
			param( const char *new_name ): named_object( new_name ) {}
			virtual ~param() {}

			//

			virtual bool set_string( const char *new_string ) = 0;
			virtual bool validate_string( const char *new_string ) = 0;			
	};

	
	// a primitive parameter can take any primitive
	// data type as value and is validated via
	// any unary predicate
	template <typename T>
	class primitive_param: public param
	{
		protected:
			T value;
			predicate<T> *val_pred;
			predicate<T> *prot_pred;
		
		public:
			primitive_param( const char *new_name, T new_value, predicate<T> *new_val_pred, predicate<T> *new_prot_pred ): param( new_name ), value( new_value ), val_pred( new_val_pred ), prot_pred( new_prot_pred ) {}
			
			virtual ~primitive_param()
			{
				delete val_pred;
				delete prot_pred;
			}
			
			//
			
			virtual char *get_string()
			{
				std::string temp_str;
				to_string( value, temp_str );
				return strdup( temp_str.c_str() );
			}

			virtual bool set_string( const char *new_string )
			{
				T new_val;
				from_string( new_val, new_string );

				if ( !(*val_pred)( new_val ) || (*prot_pred)( new_val ) )
				{
					return false;
				}
				else
				{
					set_value( new_val );
					return true;
				}
			}

			virtual bool validate_string( const char *new_string )
			{
				T new_val;
				from_string( new_val, new_string );

				return (*val_pred)( new_val );
			}
			
			//
			
			virtual T get_value()
			{
				return value;
			}

			virtual void set_value( T new_value )
			{
				value = new_value;
			}
	};
	
	// these are easy definitions for int and double parameters
	typedef primitive_param<int64_t> integer_param;
	typedef primitive_param<double> decimal_param;
	

	// a string param deals with character strings
	class string_param: public param
	{
		protected:
			std::string *value;
			predicate<const char *> *val_pred;
			predicate<const char *> *prot_pred;
		
		public:
			string_param( const char *new_name, const char *new_value, predicate<const char *> *new_val_pred, predicate<const char *> *new_prot_pred ): param( new_name ), value( new std::string( new_value ) ), val_pred( new_val_pred ), prot_pred( new_prot_pred ) {}

			virtual ~string_param()
			{
				delete value;
				delete val_pred;
				delete prot_pred;
			}
			
			//
			
			virtual char *get_string()
			{
				char *return_val = new char[ value->length() + 1 ];
				strcpy( return_val, value->c_str() );
				return_val[ value->length() ] = '\0';

				return return_val;
			}

			virtual bool set_string( const char *new_string )
			{
				if ( !(*val_pred)( new_string ) || (*prot_pred)( new_string ) )
				{
					return false;
				}
				else
				{
					set_value( new_string );
					return true;
				}
			}

			virtual bool validate_string( const char *new_value )
			{
				return (*val_pred)( new_value );
			}
			
			//
			
			virtual const char *get_value()
			{
				return value->c_str();
			}

			virtual void set_value( const char *new_value )
			{
				value->assign( new_value );
			}
	};

	// a primitive_set param maintains a set of primitives
	template <typename T>
	class primitive_set_param: public param
	{
		protected:
			std::set< T > *my_set;
			std::string *value;
			predicate< T > *prot_pred;

		public:
			primitive_set_param( const char *new_name, predicate< T > *new_prot_pred ): param( new_name ), my_set( new std::set< T >() ), value( new std::string ), prot_pred( new_prot_pred ) {}

			virtual ~primitive_set_param()
			{
				delete my_set;
				delete value;
				delete prot_pred;
			}

			virtual char *get_string()
			{
				char *return_val = new char[ value->length() + 1 ];
				strcpy( return_val, value->c_str() );
				return_val[ value->length() ] = '\0';

				return return_val;
			}

			virtual bool validate_string( const char *new_value )
			{
				T test_val;

				return from_string( test_val, new_value );
			}

			virtual bool set_string( const char *new_string )
			{
				T new_val;
				from_string( new_val, new_string );
				
				if ( (*prot_pred)( new_val ) )
				{
					return false;
				}
				else
				{
					typename std::set< T >::iterator it = my_set->find( new_val );
					std::string temp_str;

					if ( it != my_set->end() )
					{
						my_set->erase( it );

						// regenerate value from scratch
						value->clear();
						for ( it=my_set->begin(); it!=my_set->end(); )
						{
							to_string( *it, temp_str );
							value->append( temp_str );

							it++;

							if ( it != my_set->end() )
								value->append( ", " );
						}
					}
					else
					{
						my_set->insert( new_val );

						if ( !value->empty() )
							value->append( ", " );

						to_string( new_val, temp_str );
						value->append( temp_str );
					}


					return true;
				}
			}

			virtual bool in_set( T test_val )
			{
				return ( my_set->find( test_val ) != my_set->end() );
			}

			virtual typename std::set< T >::iterator set_begin()
			{
				return my_set->begin();
			}

			virtual typename std::set< T >::iterator set_end()
			{
				return my_set->end();
			}
	};

	// these are easy definitions for sets
	typedef primitive_set_param< int64_t > int_set_param;

	// a sym_set param maintains a set of strings
	class sym_set_param: public param
	{
		protected:
			std::set<Symbol *> *my_set;
			std::string *value;
			predicate<const char *> *prot_pred;

			agent *my_agent;

		public:
			sym_set_param( const char *new_name, predicate<const char *> *new_prot_pred, agent *new_agent ): param( new_name ), my_set( new std::set<Symbol *>() ), value( new std::string ), prot_pred( new_prot_pred ), my_agent( new_agent ) {}

			virtual ~sym_set_param()
			{
				for ( std::set<Symbol *>::iterator p=my_set->begin(); p!=my_set->end(); p++ )
					symbol_remove_ref( my_agent, (*p) );
				
				delete my_set;
				delete value;
				delete prot_pred;
			}

			//

			virtual char *get_string()
			{
				char *return_val = new char[ value->length() + 1 ];
				strcpy( return_val, value->c_str() );
				return_val[ value->length() ] = '\0';

				return return_val;
			}

			virtual bool set_string( const char *new_string )
			{
				if ( (*prot_pred)( new_string ) )
				{
					return false;
				}
				else
				{
					set_value( new_string );
					return true;
				}
			}

			virtual bool validate_string( const char * /*new_value*/ )
			{
				return true;
			}

			//

			virtual bool in_set( Symbol *test_sym )
			{
				bool return_val = false;

				if ( ( test_sym->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE ) ||
					 ( test_sym->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE ) ||
					 ( test_sym->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE ) )
				{
					Symbol *my_sym = test_sym;

					if ( my_sym->common.symbol_type != SYM_CONSTANT_SYMBOL_TYPE )
					{
						std::string temp_str;

						if ( my_sym->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE )
						{
							to_string( my_sym->ic.value, temp_str );
						}
						else
						{
							to_string( my_sym->fc.value, temp_str );
						}

						my_sym = make_sym_constant( my_agent, temp_str.c_str() );
					}

					std::set<Symbol *>::iterator p = my_set->find( my_sym );
					return_val = ( p != my_set->end() );

					if ( test_sym != my_sym )
					{
						symbol_remove_ref( my_agent, my_sym );
					}
				}

				return return_val;
			}

			virtual void set_value( const char *new_value )
			{
				Symbol *my_sym = make_sym_constant( my_agent, new_value );
				std::set<Symbol *>::iterator p = my_set->find( my_sym );

				if ( p != my_set->end() )
				{
					my_set->erase( p );

					// remove for now and when added to the set
					symbol_remove_ref( my_agent, my_sym );
					symbol_remove_ref( my_agent, my_sym );

					// regenerate value from scratch
					value->clear();
					for ( p=my_set->begin(); p!=my_set->end(); )
					{
						value->append( (*p)->sc.name );

						p++;

						if ( p != my_set->end() )
							value->append( ", " );
					}
				}
				else
				{
					my_set->insert( my_sym );

					if ( !value->empty() )
						value->append( ", " );

					value->append( my_sym->sc.name );
				}
			}
	};

	// a constant parameter deals in discrete values
	// for efficiency, internally we use enums, elsewhere
	// strings for user-readability
	template <typename T>
	class constant_param: public param
	{
		protected:		
			T value;
			std::map<T, const char *> *value_to_string;
			std::map<std::string, T> *string_to_value;
			predicate<T> *prot_pred;			
			
		public:						
			constant_param( const char *new_name, T new_value, predicate<T> *new_prot_pred ): param( new_name ), value( new_value ), value_to_string( new std::map<T, const char *>() ), string_to_value( new std::map<std::string, T> ), prot_pred( new_prot_pred ) {}
			
			virtual ~constant_param()
			{
				delete value_to_string;
				delete string_to_value;
				delete prot_pred;
			}
			
			//
			
			virtual char *get_string()
			{
				typename std::map<T, const char *>::iterator p;
				p = value_to_string->find( value );

				if ( p == value_to_string->end() )
					return NULL;
				else
				{
					size_t len = strlen( p->second );
					char *return_val = new char[ len + 1 ];

					strcpy( return_val, p->second );
					return_val[ len ] = '\0';

					return return_val;
				}
			}

			virtual bool set_string( const char *new_string )
			{
				typename std::map<std::string, T>::iterator p;
				std::string temp_str( new_string );

				p = string_to_value->find( temp_str );

				if ( ( p == string_to_value->end() ) || (*prot_pred)( p->second ) )
				{
					return false;
				}
				else
				{
					set_value( p->second );
					return true;
				}
			}

			virtual bool validate_string( const char *new_string )
			{
				typename std::map<std::string, T>::iterator p;
				std::string temp_str( new_string );

				p = string_to_value->find( temp_str );

				return ( p != string_to_value->end() );
			}

			//
			
			virtual T get_value()
			{
				return value;
			}

			virtual void set_value( T new_value )
			{
				value = new_value;
			}
			
			//
			
			virtual void add_mapping( T val, const char *str )
			{
				std::string my_string( str );

				// string to value
				(*string_to_value)[ my_string ] = val;

				// value to string
				(*value_to_string)[ val ] = str;
			}
	};

	// this is an easy implementation of a boolean parameter
	enum boolean { off, on };
	class boolean_param: public constant_param<boolean>
	{
		public:
			boolean_param( const char *new_name, boolean new_value, predicate<boolean> *new_prot_pred ): constant_param<boolean>( new_name, new_value, new_prot_pred )
			{
				add_mapping( off, "off" );
				add_mapping( on, "on" );
			}
	};
	

	///////////////////////////////////////////////////////////////////////////
	// Parameter Container
	///////////////////////////////////////////////////////////////////////////

	typedef object_container<param> param_container;


	///////////////////////////////////////////////////////////////////////////
	// Statistics
	///////////////////////////////////////////////////////////////////////////
	
	// all statistics have a name and
	// can be retrieved generically
	// via strings
	class stat: public named_object
	{			
		public:		
			stat( const char *new_name ): named_object( new_name ) {}
			virtual ~stat() {}
			
			//
			
			virtual void reset() = 0;
	};


	// a primitive statistic can take any primitive
	// data type as value
	template <typename T>
	class primitive_stat: public stat
	{
		private:
			T value;
			T reset_val;
			predicate<T> *prot_pred;
		
		public:
			primitive_stat( const char *new_name, T new_value, predicate<T> *new_prot_pred ): stat( new_name ), value( new_value ), reset_val( new_value ), prot_pred( new_prot_pred ) {}			
			
			virtual ~primitive_stat()
			{
				delete prot_pred;
			}
			
			//
			
			virtual char *get_string()
			{
				T my_val = get_value();

				std::string temp_str;
				to_string( my_val, temp_str );
				return strdup(temp_str.c_str());
			}

			void reset()
			{
				if ( !(*prot_pred)( value ) )
					value = reset_val;
			}
			
			//
			
			virtual T get_value()
			{
				return value;
			}

			virtual void set_value( T new_value )
			{
				value = new_value;
			}
	};

	// these are easy definitions for int and double stats
	typedef primitive_stat<int64_t> integer_stat;
	typedef primitive_stat<double> decimal_stat;	
	
	///////////////////////////////////////////////////////////////////////////
	// Statistic Containers
	///////////////////////////////////////////////////////////////////////////

	class stat_container: public object_container<stat>
	{
		public:
			stat_container( agent *new_agent ): object_container<stat>( new_agent ) {}

			//

			void reset()
			{
				for ( std::map<std::string, stat *>::iterator p=objects->begin(); p!=objects->end(); p++ )
					p->second->reset();
			}
	};


	///////////////////////////////////////////////////////////////////////////
	// timers
	///////////////////////////////////////////////////////////////////////////	

	class timer: public named_object
	{
		public:
			enum timer_level { zero, one, two, three, four, five };
	
		protected:
			agent *my_agent;
			
			soar_process_timer stopwatch;
			soar_timer_accumulator accumulator;

			timer_level level;
			predicate<timer_level> *pred;			

		public:

			timer( const char *new_name, agent *new_agent, timer_level new_level, predicate<timer_level> *new_pred );

			virtual ~timer()
			{
				delete pred;
			}

			//

			virtual char *get_string()
			{
				double my_value = value();

				std::string temp_str;
				to_string( my_value, temp_str );
				return strdup(temp_str.c_str());
			}

			//

			virtual void reset()
			{
				stopwatch.stop();
				accumulator.reset();
			}

			virtual double value()
			{
				return accumulator.get_sec();
			}

			//

			virtual void start()
			{
				if ( (*pred)( level ) )
				{
					stopwatch.start();
				}
			}

			virtual void stop()
			{
				if ( (*pred)( level ) )
				{
					stopwatch.stop();
					accumulator.update(stopwatch);
				}
			}
	};


	///////////////////////////////////////////////////////////////////////////
	// Timer Containers
	///////////////////////////////////////////////////////////////////////////

	class timer_container: public object_container<timer>
	{
		public:
			timer_container( agent *new_agent ): object_container<timer>( new_agent ) {}

			//

			void reset()
			{
				for ( std::map<std::string, timer *>::iterator p=objects->begin(); p!=objects->end(); p++ )
					p->second->reset();
			}
	};


	///////////////////////////////////////////////////////////////////////////
	// Memory Pool Allocators
	///////////////////////////////////////////////////////////////////////////

#define USE_MEM_POOL_ALLOCATORS 1

#ifdef USE_MEM_POOL_ALLOCATORS

	memory_pool* get_memory_pool( agent* my_agent, size_t size );

	template <class T>
	class soar_memory_pool_allocator
	{
	public:
		typedef T			value_type;
		typedef size_t		size_type;
		typedef ptrdiff_t	difference_type;

		typedef T*			pointer;
		typedef const T*	const_pointer;

		typedef T&			reference;
		typedef const T&	const_reference;

	public:
		agent* get_agent() const { return my_agent; }

		soar_memory_pool_allocator( agent* new_agent ): my_agent(new_agent), mem_pool(NULL), size(sizeof(value_type))
		{
			// useful for debugging
			// std::string temp_this( typeid( value_type ).name() );
		}

		soar_memory_pool_allocator( const soar_memory_pool_allocator& obj ): my_agent(obj.get_agent()), mem_pool(NULL), size(sizeof(value_type))
		{
			// useful for debugging
			// std::string temp_this( typeid( value_type ).name() );
		}

		template <class _other>
		soar_memory_pool_allocator( const soar_memory_pool_allocator<_other>& other ): my_agent(other.get_agent()), mem_pool(NULL), size(sizeof(value_type))
		{
			// useful for debugging
			// std::string temp_this( typeid( T ).name() );
			// std::string temp_other( typeid( _other ).name() );
		}

		pointer allocate( size_type n, const void* = 0 )
		{
			size_type test = n;
			test; // prevents release-mode warning, since assert is compiled out
			assert( test == 1 );
			
			if ( !mem_pool )
			{
				mem_pool = get_memory_pool( my_agent, size );
			}
			
			pointer t;
			allocate_with_pool( my_agent, mem_pool, &t );

			return t;
		}

		void deallocate( void* p, size_type n )
		{
			size_type test = n;
			test; // prevents release-mode warning, since assert is compiled out
			assert( test == 1 );

			// not sure if this is correct...
			// it only comes up if an object uses another object's
			// allocator to deallocate memory that it allocated.
			// it's quite possible, then, that the sizes would be off
			if ( !mem_pool )
			{
				mem_pool = get_memory_pool( my_agent, size );
			}
			
			if ( p )
			{
				free_with_pool( mem_pool, p );
			}
		}

		void construct( pointer p, const_reference val )
		{
			new (p) T( val );
		}

		void destroy( pointer p )
		{
			p->~T();
		}

		size_type max_size() const
		{
			return static_cast< size_type >( -1 );
		}

		const_pointer address( const_reference r ) const
		{
			return &r;
		}

		pointer address( reference r ) const
		{
			return &r;
		}

		template <class U>
		struct rebind
		{
			typedef soar_memory_pool_allocator<U> other;
		};


	private:
		agent* my_agent;
		memory_pool* mem_pool;
		size_type size;

		soar_memory_pool_allocator() {}

	};

#endif

}

#endif
