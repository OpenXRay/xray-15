#ifndef	_NET_EXECUTION_FACTORY_
#define	_NET_EXECUTION_FACTORY_

#include "net_execution.h"

namespace lc_net{

	class base_execution_type_creator
	{
		public:
		virtual net_execution* create	( u32 _net_id ) = 0;
		virtual	u32			   type		( ) = 0;

	};




	class factory
	{
		
	public:
		//typedef std::pair<u32, base_execution_type_creator*> type_reg;
		typedef  base_execution_type_creator* type_reg;
	private:
		xr_vector< type_reg > vec_types;
	public:
			net_execution			*create( u32 type_id, u32 _net_id );

			template < execution_types etype >
			tnet_execution_base< etype >	*create(  )
			{
				return (tnet_execution_base< etype >*) create( etype, u32(-1) );
			}


			factory					();
			~factory				();
	


		public:
			void							register_type	( base_execution_type_creator* creator );
		private:
		//	xr_vector< type_reg >::iterator find_type		( u32 id  );
			void							register_all	();
			void							clear			();

	};

extern factory execution_factory;
};
#endif