package com.twicestruck.jqc.vm;

public class Header
{
	public int version; //KrimZon: should be 6
	public int crc;

	public int statements_offset;
	public int statements_num;

	public int globaldefs_offset;
	public int globaldefs_num;

	public int fielddefs_offset;
	public int fielddefs_num;

	public int functions_offset;
	public int functions_num;

	public int stringdata_offset;
	public int stringdata_size;

	public int globaldata_offset;
	public int globaldata_num;

	public int entity_size; //KrimZon: number of 32 bit words
}
