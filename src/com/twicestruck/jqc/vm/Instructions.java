package com.twicestruck.jqc.vm;

public class Instructions
{
	public static final byte DONE       = 0x0000;

	public static final byte MUL_F      = 0x0001;
	public static final byte MUL_V      = 0x0002;
	public static final byte MUL_FV     = 0x0003;
	public static final byte MUL_VF     = 0x0004;
	public static final byte DIV_F      = 0x0005;
	public static final byte ADD_F      = 0x0006;
	public static final byte ADD_V      = 0x0007;
	public static final byte SUB_F      = 0x0008;
	public static final byte SUB_V      = 0x0009;

	public static final byte EQ_F       = 0x000A;
	public static final byte EQ_V       = 0x000B;
	public static final byte EQ_S       = 0x000C;
	public static final byte EQ_E       = 0x000D;
	public static final byte EQ_FNC     = 0x000E;

	public static final byte NE_F       = 0x000F;
	public static final byte NE_V       = 0x0010;
	public static final byte NE_S       = 0x0011;
	public static final byte NE_E       = 0x0012;
	public static final byte NE_FNC     = 0x0013;

	public static final byte LE         = 0x0014;
	public static final byte GE         = 0x0015;
	public static final byte LT         = 0x0016;
	public static final byte GT         = 0x0017;

	public static final byte LOAD_F     = 0x0018;
	public static final byte LOAD_V     = 0x0019;
	public static final byte LOAD_S     = 0x001A;
	public static final byte LOAD_ENT   = 0x001B;
	public static final byte LOAD_FLD   = 0x001C;
	public static final byte LOAD_FNC   = 0x001D;

	public static final byte ADDRESS    = 0x001E;

	public static final byte STORE_F    = 0x001F;
	public static final byte STORE_V    = 0x0020;
	public static final byte STORE_S    = 0x0021;
	public static final byte STORE_ENT  = 0x0022;
	public static final byte STORE_FLD  = 0x0023;
	public static final byte STORE_FNC  = 0x0024;

	public static final byte STOREP_F   = 0x0025;
	public static final byte STOREP_V   = 0x0026;
	public static final byte STOREP_S   = 0x0027;
	public static final byte STOREP_ENT = 0x0028;
	public static final byte STOREP_FLD = 0x0029;
	public static final byte STOREP_FNC = 0x002A;

	public static final byte RETURN     = 0x002B;

	public static final byte NOT_F      = 0x002C;
	public static final byte NOT_V      = 0x002D;
	public static final byte NOT_S      = 0x002E;
	public static final byte NOT_ENT    = 0x002F;
	public static final byte NOT_FNC    = 0x0030;

	public static final byte IF         = 0x0031;
	public static final byte IFNOT      = 0x0032;

	public static final byte CALL0      = 0x0033;
	public static final byte CALL1      = 0x0034;
	public static final byte CALL2      = 0x0035;
	public static final byte CALL3      = 0x0036;
	public static final byte CALL4      = 0x0037;
	public static final byte CALL5      = 0x0038;
	public static final byte CALL6      = 0x0039;
	public static final byte CALL7      = 0x003A;
	public static final byte CALL8      = 0x003B;

	public static final byte STATE      = 0x003C;

	public static final byte GOTO       = 0x003D;

	public static final byte AND        = 0x003E;
	public static final byte OR         = 0x003F;

	public static final byte BITAND     = 0x0040;
	public static final byte BITOR      = 0x0041;

	// these aren't instructions, but shorthand for the range
	public static final byte MIN        = 0x0000;
	public static final byte MAX        = 0x0041;
}
