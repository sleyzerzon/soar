<?php

	// tabs  = array( id=>title )
	// callback = 'function_name' || array( object, 'function_name' ) : ( $id )
	function report_create_tabs( $tab_id, $tabs, $callback )
	{
		global $page_info;
		
		$page_info['head'] .= jquery_tabs( $tab_id );
		
		echo ( '<div id="' . htmlentities( $tab_id ) . '">' );
		
			echo '<ul>';
			foreach ( $tabs as $tk => $tv )
			{
				echo ( '<li><a href="#' . htmlentities( $tk ) . '">' . htmlentities( $tv ) . '</a></li>' );
			}
			echo '</ul>';
		
			foreach ( $tabs as $tk => $tv )
			{
				echo ( '<div id="' . htmlentities( $tk ) . '">' );
				
					call_user_func_array( $callback, array( $tk ) );
				
				echo '</div>';
			}
		
		echo '</div>';
	}
	
	class report_data
	{
		private $data;
		
		function __construct()
		{
			$this->data = array();
		}
		
		public function set_exists( $id )
		{			
			return isset( $this->data[ $id ] );
		}
		
		public function add_set( $id, $sql, $schema )
		{
			if ( !$this->set_exists( $id ) )
			{
				$this->data[ $id ] = array( 'sql'=>$sql, 'schema'=>$schema );
			}
		}
		
		public function get_sql( $id )
		{
			$return_val = NULL;
			
			if ( $this->set_exists( $id ) )
			{
				$return_val = $this->data[ $id ]['sql'];
			}
			
			return $return_val;
		}
		
		public function get_schema( $id )
		{
			$return_val = NULL;
			
			if ( $this->set_exists( $id ) )
			{
				$return_val = $this->data[ $id ]['schema'];
			}
			
			return $return_val;
		}
		
		public function get_data( $id )
		{
			$return_val = NULL;
			
			if ( $this->set_exists( $id ) )
			{
				if ( !isset( $this->data[ $id ]['table'] ) )
				{
					global $db;
					
					$this->data[ $id ]['table'] = array();
					$res = @mysql_query( $this->data[ $id ]['sql'], $db );
					while ( $row = mysql_fetch_assoc( $res ) )
					{
						foreach ( $this->data[ $id ]['schema'] as $k => $v )
						{
							if ( ( $v == EXP_TYPE_INT ) && ( isset( $row[ $k ] ) ) )
							{
								$row[ $k ] = intval( $row[ $k ] );
							}
							else if ( ( $v == EXP_TYPE_DOUBLE ) && ( isset( $row[ $k ] ) ) )
							{
								$row[ $k ] = doubleval( $row[ $k ] );
							}
						} 
						
						$this->data[ $id ]['table'][] = $row;
					}
				}
				
				$return_val =& $this->data[ $id ]['table'];
			}
			
			return $return_val;
		}
	
		// returns phplot-friendly data array
		public function convert_data( $id, $x_field, $y_field, $set_field = NULL )
		{
			$return_val = NULL;
			
			$table = $this->get_data( $id );
			if ( !is_null( $table ) )
			{
				$temp = array( 'sets'=>array(), 'data'=>array() );
				$return_val = array( 
					'data'=>array(),
					'stats' => array(
						'x-min' => NULL,
						'x-max' => NULL,
						'y-min' => NULL,
						'y-max' => NULL,
					),
				);
				
				// populate existing data
				foreach ( $table as $row )
				{
					$set = ( ( is_null( $set_field ) )?( 'uniform' ):( $row[ $set_field ] ) );
					$x = $row[ $x_field ];
					$y = $row[ $y_field ];
					
					if ( !isset( $temp['sets'][ $set ] ) )
					{
						$temp['sets'][ $set ] = $set;
					}
					
					if ( !isset( $temp['data'][ $x ] ) )
					{
						$temp['data'][ $x ] = array();
						
						// maintain stats
						if ( $this->data[ $id ]['schema'][ $x_field ] != EXP_TYPE_STRING )
						{
							if ( is_null( $return_val['stats']['x-min'] ) || ( $return_val['stats']['x-min'] > $x ) )
							{
								$return_val['stats']['x-min'] = $x;
							}
							
							if ( is_null( $return_val['stats']['x-max'] ) || ( $return_val['stats']['x-max'] < $x ) )
							{
								$return_val['stats']['x-max'] = $x;
							}
						}
					}
					
					$temp['data'][ $x ][ $set ] = $y;
					
					// maintain stats
					{
						if ( is_null( $return_val['stats']['y-min'] ) || ( $return_val['stats']['y-min'] > $y ) )
						{
							$return_val['stats']['y-min'] = $y;
						}
						
						if ( is_null( $return_val['stats']['y-max'] ) || ( $return_val['stats']['y-max'] < $y ) )
						{
							$return_val['stats']['y-max'] = $y;
						}
					}
				}
				
				// fill in missing data
				foreach ( $temp['data'] as $x => $row )
				{
					foreach ( $temp['sets'] as $set_name )
					{
						if ( !isset( $row[ $set_name ] ) )
						{
							$temp['data'][ $x ][ $set_name ] = ''; 
						}
					}
				}
				
				// sort y-data by sets
				ksort( $temp['sets'] );
				foreach ( $temp['data'] as $x => $row )
				{
					ksort( $row );
					
					$temp['data'][ $x ] = $row;
				}
				
				// sort x-data
				ksort( $temp['data'] );
				
				// make phplot format
				{
					$prefix = array();
					
					if ( $this->data[ $id ]['schema'][ $x_field ] != EXP_TYPE_STRING )
					{
						$prefix[] = '';
					}
					
					foreach ( $temp['data'] as $x => $row )
					{
						$phplot_row = $prefix;
						
						// add x-label
						$phplot_row[] = $x;
						
						foreach ( $row as $y )
						{
							$phplot_row[] = $y;
						}
						
						$return_val['data'][] = $phplot_row;
					}
				}
				
				if ( !is_null( $set_field ) )
				{
					$return_val['sets'] = array_keys( $temp['sets'] );
				}
			}
			
			return $return_val;
		}
	}
	
?>
