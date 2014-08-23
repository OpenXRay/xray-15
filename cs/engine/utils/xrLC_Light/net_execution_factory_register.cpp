#include "stdafx.h"
#include "net_execution_factory.h"

#include "net_execution_lightmaps.h"


namespace lc_net{


		template < execution_types etype > class tnet_execution :
		public tnet_execution_base< etype >
	{
	public:
		tnet_execution	( u32 id ): tnet_execution_base< etype >(id){}
	private:
		net_execution_impl					execution_impl;

		virtual		net_execution_impl	&implementation	( ) 
		{
			return execution_impl;
		};

		virtual		void					send_task		( IGridUser& user, IGenericStream* outStream, u32  id )	 
		{
			tnet_execution_base< etype >::send_task( user, outStream, id );
			execution_impl.send_task( user, outStream, id );
		};
		virtual		LPCSTR					data_files		()
		{
			return execution_impl.data_files();
		}
		virtual		void					receive_result	( IGenericStream* outStream )	
		{
			execution_impl.receive_result(outStream);
		};
		virtual		bool					receive_task	( IAgent* agent, DWORD sessionId, IGenericStream* inStream ) 
		{
			return execution_impl.receive_task( agent, sessionId, inStream );
		};
		virtual		void					send_result		( IGenericStream* outStream )	
		{
			execution_impl.send_result( outStream );
		};
		virtual		bool					execute			()	
		{
			return execution_impl.execute			();
		};
	};
	


	//template < execution_types etype >
	//tnet_execution_base< etype >	*factory::create( const execution_types etype )
	//{
	//	return xr_new< tnet_execution< etype > > ();
	//}
	
	template<typename execution > 
	class execution_type_creator: 
		public base_execution_type_creator
	{
		//static const u32 class_type = execution::class_type;

		virtual	net_execution* create( u32 _net_id )
		{
			return xr_new<execution>(_net_id);
		}
		virtual	u32 type() { return execution::class_type; }
	};



	template<typename execution>
	static void	register_type()
	{
		execution_factory.register_type(  xr_new< execution_type_creator<execution> >() );
	}


	template < execution_types i >
	struct it
	{
		static const execution_types et =	   (execution_types)(i);
		static const execution_types next_et = (execution_types)(i+1);
		typedef	it<next_et> next;
		next ni;
		it(){ register_type< tnet_execution< et > >();  }

	} ;

	template<> struct it<et_last>
	{};


	void	factory::register_all( )
	{
		vec_types.resize( et_last, 0 );
		it< et_lightmaps > i;

		//static const execution_types et =  it<et_lightmaps>::et ;
		//lc_net::register_type< tnet_execution< et > >(  );
	}


};