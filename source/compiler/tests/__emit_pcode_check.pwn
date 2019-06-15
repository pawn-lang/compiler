#include "__emit.inc"

// Use 'push.r' as a delimiter since the compiler doesn't generate this instruction
// and its argument might be used to identify the snippet in the disassembly.
#define DEF_SNIPPET(%0) emit push.r (%0)

const global_const_1 = 1;
const global_const_1234 = 0x1234;


stock test__pcode(&local_refvar, local_refarray[])
{
	const local_const_1 = 1;
	const local_const_5678 = 0x5678;
	static local_static_var = 0;
	static local_static_array[2];
	new local_var = 0;
	new local_array[2];

	DEF_SNIPPET(0x0);
	emit load.pri global_var;			// load.pri 00000000
	emit load.pri global_array;			// load.pri 00000004
	emit load.pri local_static_var;		// load.pri 0000000c

	DEF_SNIPPET(0x1);
	emit load.s.pri global_const_1234;	// load.s.pri 00001234
	emit load.s.pri local_refvar;		// load.s.pri 0000000c
	emit load.s.pri local_refarray;		// load.s.pri 00000010
	emit load.s.pri local_const_5678;	// load.s.pri 00005678
	emit load.s.pri local_var;			// load.s.pri fffffffc
	emit load.s.pri 0xFEDC;				// load.s.pri 0000fedc

	DEF_SNIPPET(0x2);
	emit call global_func;				// call 00000008
	emit sysreq.c global_native;		// sysreq.c 00000000
	emit sysreq.n global_native 0;		// push.c 00000000 \ sysreq.c 00000000 \ stack 00000004

	DEF_SNIPPET(0x3);
	emit lodb.i global_const_1;			// lodb.i 00000001
	emit lodb.i local_const_1;			// lodb.i 00000001
	emit lodb.i 1;						// lodb.i 00000001
	emit lodb.i 2;						// lodb.i 00000002
	emit lodb.i 4;						// lodb.i 00000004
	emit lodb.i 0x1;					// lodb.i 00000001
	emit lodb.i 0x2;					// lodb.i 00000002
	emit lodb.i 0x4;					// lodb.i 00000004

	DEF_SNIPPET(0x4);
	emit align.pri global_const_1;		// align.pri 00000001
	emit align.pri local_const_1;		// align.pri 00000001
	emit align.pri 0;					// align.pri 00000000
	emit align.pri 1;					// align.pri 00000001
	emit align.pri 2;					// align.pri 00000002
	emit align.pri 3;					// align.pri 00000003
	emit align.pri 0x0;					// align.pri 00000000
	emit align.pri 0x1;					// align.pri 00000001
	emit align.pri 0x2;					// align.pri 00000002
	emit align.pri 0x3;					// align.pri 00000003

	DEF_SNIPPET(0x5);
	// push.c 00001234 \ push.c 00005678 \ push.c 00009abc
	emit push3.c global_const_1234 local_const_5678 0x9ABC;
	// push.c push.c 00000000 \ push.c 00000004 \ push.c 0000000c \ push.c 0000000c \ push.c fffffffc
	emit push5.c global_var global_array local_static_var local_refvar local_var;

	DEF_SNIPPET(0x6);
	// push 00000000 \ push 0000000c
	emit push2 global_var local_static_var;

	DEF_SNIPPET(0x7);
	// push.s 00001234 \ push.s 00005678 \ push.s 0000000c \ push.s 00000010 \ push.s fffffffc
	emit push5.s global_const_1234 local_const_5678 local_refvar local_refarray local_var;

	DEF_SNIPPET(0x8);
	// push.adr 00001234 \ push.adr 00005678 \ push.adr 0000000c \ push.adr 00000010 \ push.adr fffffffc
	emit push5.adr global_const_1234 local_const_5678 local_refvar local_refarray local_var;

	DEF_SNIPPET(0x9);
	// push.pri \ const.pri 00001234 \ stor.pri 00000000 \ pop.pri
	emit const global_var global_const_1234;
	// push.pri \ const.pri 00005678 \ stor.pri 0000000c \ pop.pri
	emit const local_static_var local_const_5678;

	DEF_SNIPPET(0xa);
	// push.pri \ const.pri 00005678 \ stor.s.pri fffffffc \ pop.pri
	emit const.s local_var local_const_5678;

	DEF_SNIPPET(0xb);
	// load.pri 00000000 \ load.alt 0000000c
	emit load.both global_var local_static_var;

	DEF_SNIPPET(0xc);
	// load.pri 0000000c \ load.alt fffffffc
	emit load.s.both local_refvar local_var;

	DEF_SNIPPET(0x10);
	emit addr.u.pri global_var;			// zero.pri
	emit addr.u.alt global_var;			// zero.alt
	emit addr.u.pri local_refvar;		// load.s.pri 0000000c
	emit addr.u.alt local_refvar;		// load.s.alt 0000000c
	emit addr.u.pri local_var;			// addr.pri fffffffc
	emit addr.u.alt local_var;			// addr.alt fffffffc
	emit addr.u.pri local_static_var;	// const.pri 0000000c
	emit addr.u.alt local_static_var;	// const.alt 0000000c
	DEF_SNIPPET(0x11);
	emit addr.u.pri global_array[0];	// const.pri 00000004
	emit addr.u.alt global_array[1];	// const.alt 00000008
	emit addr.u.pri local_static_array[0];	// const.pri 00000010
	emit addr.u.alt local_static_array[1];	// const.alt 00000014
	emit addr.u.pri local_array[0];		// addr.pri fffffff4
	emit addr.u.alt local_array[1];		// addr.alt fffffff8
	emit addr.u.pri local_refarray[0];	// load.s.pri 00000010
	emit addr.u.alt local_refarray[1];	// load.s.alt 00000014
	DEF_SNIPPET(0x12);
	emit addr.u.pri global_array{0};	// const.pri 00000004 \ align.pri 00000001
	emit addr.u.alt global_array{1};	// const.alt 00000005 \ align.alt 00000001
	emit addr.u.pri local_static_array{0};	// const.pri 00000010 \ align.pri 00000001
	emit addr.u.alt local_static_array{1};	// const.alt 00000011 \ align.alt 00000001
	emit addr.u.pri local_array{0};		// addr.pri fffffff4 \ align.pri 00000001
	emit addr.u.alt local_array{1};		// addr.alt fffffff5 \ align.alt 00000001
	DEF_SNIPPET(0x13);
	emit addr.u.pri local_refarray{0};	// load.s.pri 00000010 \ align.pri 00000001
	emit addr.u.alt local_refarray{0};	// load.s.alt 00000010 \ align.alt 00000001
	emit addr.u.pri local_refarray{1};	// load.s.pri fffffff4 \ inc.pri \ align.pri 00000001
	emit addr.u.alt local_refarray{1};	// load.s.alt fffffff4 \ inc.alt \ align.alt 00000001
	emit addr.u.pri local_refarray{2};	// load.s.pri fffffff5 \ add.c 00000002 \ align.pri 00000001
	emit addr.u.alt local_refarray{2};	// load.s.pri fffffff5 \ add.c 00000002 \ move.alt \ align.alt 00000001
	DEF_SNIPPET(0x14);
	emit addr.u.pri global_array[local_var];		// load.s.pri fffffffc \ const.alt 00000004 \ idxaddr
	emit addr.u.alt global_array[local_var];		// load.s.pri fffffffc \ const.alt 00000004 \ idxaddr \ move.alt
	emit addr.u.pri local_static_array[local_var];	// load.s.pri fffffffc \ const.alt 00000010 \ idxaddr
	emit addr.u.alt local_static_array[local_var];	// load.s.pri fffffffc \ const.alt 00000010 \ idxaddr \ move.alt
	emit addr.u.pri local_array[local_var];			// load.s.pri fffffffc \ addr.alt fffffff4 \ idxaddr
	emit addr.u.alt local_array[local_var];			// load.s.pri fffffffc \ addr.alt fffffff4 \ idxaddr \ move.alt
	emit addr.u.pri local_refarray[local_var];		// load.s.pri fffffffc \ load.s.alt 00000010 \ idxaddr
	emit addr.u.alt local_refarray[local_var];		// load.s.pri fffffffc \ load.s.alt 00000010 \ idxaddr \ move.alt
	DEF_SNIPPET(0x15);
	emit addr.u.pri global_array{local_var};		// load.s.pri fffffffc \ add.c 00000004 \ align.pri 00000001
	emit addr.u.alt global_array{local_var};		// load.s.pri fffffffc \ add.c 00000004 \ align.pri 00000001 \ move.alt
	emit addr.u.pri local_static_array{local_var};	// load.s.pri fffffffc \ add.c 00000010 \ align.pri 00000001
	emit addr.u.alt local_static_array{local_var};	// load.s.pri fffffffc \ add.c 00000010 \ align.pri 00000001 \ move.alt
	emit addr.u.pri local_array{local_var};			// load.s.pri fffffffc \ addr.alt fffffff4 \ add \ align.pri 00000001
	emit addr.u.alt local_array{local_var};			// load.s.pri fffffffc \ addr.alt fffffff4 \ add \ align.pri 00000001 \ move.alt
	emit addr.u.pri local_refarray{local_var};		// load.s.pri fffffffc \ load.s.alt 00000010 \ add \ align.pri 00000001
	emit addr.u.alt local_refarray{local_var};		// load.s.pri fffffffc \ load.s.alt 00000010 \ add \ align.pri 00000001 \ move.alt
	DEF_SNIPPET(0x16);
	emit addr.u.pri global_array[global_func()];	// push.c 0 \ call global_func \ const.alt 00000004 \ idxaddr
	emit addr.u.alt global_array[global_func()];	// push.c 0 \ call global_func \ const.alt 00000004 \ idxaddr \ move.alt
	DEF_SNIPPET(0x17);
	emit addr.u.pri global_array{global_func()};	// push.c 0 \ call global_func \ add.c 00000004 \ align.pri 00000001
	emit addr.u.alt global_array{global_func()};	// push.c 0 \ call global_func \ add.c 00000004 \ align.pri 00000001 \ move.alt

	DEF_SNIPPET(0x18);
	emit load.u.pri global_const_1234;	// const.pri 00001234
	emit load.u.pri global_var;			// load.pri 00000000
	emit load.u.pri local_refvar;		// lref.s.pri 0000000c
	emit load.u.pri local_const_5678;	// const.pri 00005678
	emit load.u.pri local_var;			// load.s.pri fffffffc
	emit load.u.pri local_static_var;	// load.pri 0000000c
	emit load.u.pri 0x9abc;				// const.pri 00009abc
	DEF_SNIPPET(0x19);
	emit load.u.pri global_const_1234 + local_const_5678;	// const.pri 000068ac
	DEF_SNIPPET(0x1a);
	emit load.u.alt global_var * local_var;	// load.pri 00000000 \ push.pri \ load.s.pri fffffffc \ pop.alt \ smul \ move.alt
	DEF_SNIPPET(0x1b);
	emit load.u.pri local_refarray[0];	// load.s.pri 00000010 \ load.i

	DEF_SNIPPET(0x20);
	emit stor.u.pri global_var;			// stor.pri 00000000
	emit stor.u.alt global_var;			// stor.alt 00000000
	emit stor.u.pri local_refvar;		// sref.s.pri 0000000c
	emit stor.u.alt local_refvar;		// sref.s.alt 0000000c
	emit stor.u.pri local_var;			// stor.s.pri fffffffc
	emit stor.u.alt local_var;			// stor.s.alt fffffffc
	emit stor.u.pri local_static_var;	// stor.pri 0000000c
	emit stor.u.alt local_static_var;	// stor.alt 0000000c
	DEF_SNIPPET(0x21);
	emit stor.u.pri global_array[0];	// stor.pri 00000004
	emit stor.u.alt global_array[1];	// stor.alt 00000008
	emit stor.u.pri local_array[0];		// stor.s.pri fffffff4
	emit stor.u.alt local_array[1];		// stor.s.alt fffffff8
	emit stor.u.pri local_refarray[0];	// sref.s.pri 00000010
	emit stor.u.pri local_refarray[1];	// move.alt \ load.s.pri 00000010 \ add.c 00000004 \ xchg \ stor.i
	emit stor.u.alt local_refarray[1];	// load.s.pri 00000010 \ add.c 00000004 \ xchg \ stor.i
	DEF_SNIPPET(0x22);
	emit stor.u.pri global_array{0};	// const.alt 00000004 \ align.alt 00000001 \ strb.i 00000001
	emit stor.u.alt global_array{0};	// move.pri \ const.alt 00000004 \ align.alt 00000001 \ strb.i 00000001
	emit stor.u.pri global_array{1};	// const.alt 00000005 \ align.alt 00000001 \ strb.i 00000001
	emit stor.u.alt global_array{1};	// move.pri \ const.alt 00000005 \ align.alt 00000001 \ strb.i 00000001
	DEF_SNIPPET(0x23);
	emit stor.u.pri local_array{0};		// addr.alt fffffff4 \ align.alt 00000001 \ strb.i 00000001
	emit stor.u.alt local_array{0};		// move.pri \ addr.alt fffffff4 \ align.alt 00000001 \ strb.i 00000001
	emit stor.u.pri local_array{1};		// addr.alt fffffff5 \ align.alt 00000001 \ strb.i 00000001
	emit stor.u.alt local_array{1};		// move.pri \ addr.alt fffffff5 \ align.alt 00000001 \ strb.i 00000001
	DEF_SNIPPET(0x24);
	emit stor.u.pri local_refarray{0};	// load.s.alt 00000010 \ align.alt 00000001 \ strb.i 00000001
	emit stor.u.alt local_refarray{0};	// move.pri \ load.s.alt 00000010 \ align.alt 00000001 \ strb.i 00000001
	emit stor.u.pri local_refarray{1};	// load.s.alt 00000010 \ inc.alt \ align.alt 00000001 \ strb.i 00000001
	emit stor.u.alt local_refarray{1};	// move.pri \ load.s.alt 00000010 \ inc.alt \ align.alt 00000001 \ strb.i 00000001
	emit stor.u.pri local_refarray{2};	// move.alt \ load.s.pri 00000010 \ add.c 00000002 \ xchg \ align.alt 00000001 \ strb.i 00000001
	emit stor.u.alt local_refarray{2};	// load.s.pri 00000010 \ add.c 00000002 \ xchg \ align.alt 00000001 \ strb.i 00000001
	DEF_SNIPPET(0x25);
	emit stor.u.pri global_array[local_var];	// push.pri \ load.s.pri fffffffc \ const.alt 00000004 \ idxaddr \ move.alt \ pop.pri \ stor.i
	emit stor.u.alt global_array[local_var];	// push.alt \ load.s.pri fffffffc \ const.alt 00000004 \ idxaddr \ move.alt \ pop.pri \ stor.i
	emit stor.u.pri local_array[local_var];		// push.pri \ load.s.pri fffffffc \ addr.alt fffffff4 \ idxaddr \ move.alt \ pop.pri \ stor.i
	emit stor.u.alt local_array[local_var];		// push.alt \ load.s.pri fffffffc \ addr.alt fffffff4 \ idxaddr \ move.alt \ pop.pri \ stor.i
	emit stor.u.pri local_refarray[local_var];	// push.pri \ load.s.pri fffffffc \ load.s.alt 00000010 \ idxaddr \ move.alt \ pop.pri \ stor.i
	emit stor.u.alt local_refarray[local_var];	// push.alt \ load.s.pri fffffffc \ load.s.alt 00000010 \ idxaddr \ move.alt \ pop.pri \ stor.i
	DEF_SNIPPET(0x26);
	emit stor.u.pri global_array{local_var};	// push.pri \ load.s.pri fffffffc \ add.c 00000004 \ align.pri 00000001 \ move.alt \ pop.pri \ strb.i 00000001
	emit stor.u.alt global_array{local_var};	// push.alt \ load.s.pri fffffffc \ add.c 00000004 \ align.pri 00000001 \ move.alt \ pop.pri \ strb.i 00000001
	emit stor.u.pri local_array{local_var};		// push.pri \ load.s.pri fffffffc \ addr.alt fffffff4 \ add \ align.pri 00000001 \ move.alt \ pop.pri \ strb.i 00000001
	emit stor.u.alt local_array{local_var};		// push.alt \ load.s.pri fffffffc \ addr.alt fffffff4 \ add \ align.pri 00000001 \ move.alt \ pop.pri \ strb.i 00000001
	emit stor.u.pri local_refarray{local_var};	// push.pri \ load.s.pri fffffffc \ load.s.alt 00000010 \ add \ align.pri 00000001 \ move.alt \ pop.pri \ strb.i 00000001
	emit stor.u.alt local_refarray{local_var};	// push.alt \ load.s.pri fffffffc \ load.s.alt 00000010 \ add \ align.pri 00000001 \ move.alt \ pop.pri \ strb.i 00000001

	DEF_SNIPPET(0x28);
	emit push.u global_const_1234;	// push.c 00001234
	emit push.u global_var;			// push 00000000
	emit push.u local_refvar;		// lref.s.pri 0000000c \ push.pri
	emit push.u local_const_5678;	// push.c 00005678
	emit push.u local_var;			// push.s fffffffc
	emit push.u local_static_var;	// push 0000000c
	emit push.u 0x9abc;				// push.c 00009abc
	DEF_SNIPPET(0x29);
	emit push.u global_const_1234 + local_const_5678;	// push.c 000068ac
	DEF_SNIPPET(0x2a);
	emit push.u global_var * local_var;	// load.pri 00000000 \ push.pri \ load.s.pri fffffffc \ pop.alt \ smul \ push.pri
	DEF_SNIPPET(0x2b);
	emit push.u local_refarray[0];	// load.s.pri 00000010 \ load.i \ push.pri

	DEF_SNIPPET(0x30);
	emit push.u.adr global_var;			// push.c 00000000
	emit push.u.adr local_static_var;	// push.c 0000000c
	emit push.u.adr local_var;			// push.adr fffffffc
	emit push.u.adr local_refvar;		// push.s 0000000c
	DEF_SNIPPET(0x31);
	emit push.u.adr global_array[0];		// push.c 00000004
	emit push.u.adr global_array[1];		// push.c 00000008
	emit push.u.adr local_static_array[0];	// push.c 00000010
	emit push.u.adr local_static_array[1];	// push.c 00000014
	emit push.u.adr local_array[0];			// push.adr fffffff4
	emit push.u.adr local_array[1];			// push.adr fffffff8
	emit push.u.adr local_refarray[0];		// push.s 00000010
	emit push.u.adr local_refarray[1];		// load.s.pri 00000010 \ add.c 00000004 \ push.pri
	DEF_SNIPPET(0x32);
	emit push.u.adr global_array[local_var];		// load.s.pri fffffffc \ const.alt 00000004 \ idxaddr \ push.pri
	emit push.u.adr local_static_array[local_var];	// load.s.pri fffffffc \ const.alt 00000010 \ idxaddr \ push.pri
	emit push.u.adr local_array[local_var];			// load.s.pri fffffffc \ addr.alt fffffff4 \ idxaddr \ push.pri
	emit push.u.adr local_refarray[local_var];		// load.s.pri fffffffc \ load.s.alt 00000010 \ idxaddr \ push.pri
	// DEF_SNIPPET(0x33);
	// emit push.u.adr global_array{0};		// const.pri 00000004 \ align.pri 00000001 \ push.pri
	// emit push.u.adr global_array{1};		// const.pri 00000005 \ align.pri 00000001 \ push.pri
	// emit push.u.adr local_static_array{0};	// const.pri 00000010 \ align.pri 00000001 \ push.pri
	// emit push.u.adr local_static_array{1};	// const.pri 00000011 \ align.pri 00000001 \ push.pri
	// emit push.u.adr local_array{0};			// addr.pri fffffff4 \ align.pri 00000001 \ push.pri
	// emit push.u.adr local_array{1};			// addr.pri fffffff5 \ align.pri 00000001 \ push.pri
	// emit push.u.adr local_refarray{0};		// load.s.pri 00000010 \ align.pri 00000001 \ push.pri
	// emit push.u.adr local_refarray{1};		// load.s.pri 00000010 \ inc.pri \ align.pri 00000001 \ push.pri
	// emit push.u.adr local_refarray{2};		// load.s.pri 00000010 \ add.c 00000002 \ align.pri 00000001 \ push.pri
	// DEF_SNIPPET(0x34);
	// emit push.u.adr global_array{local_var};		// load.s.pri fffffffc \ add.c 00000004 \ align.pri 00000001 \ push.pri
	// emit push.u.adr local_static_array{local_var};	// load.s.pri fffffffc \ add.c 00000010 \ align.pri 00000001 \ push.pri
	// emit push.u.adr local_array{local_var};			// load.s.pri fffffffc \ addr.alt fffffff4 \ add \ align.pri 00000001 \ push.pri
	// emit push.u.adr local_refarray{local_var};		// load.s.pri fffffffc \ load.s.alt 00000010 \ add \ align.pri 00000001 \ push.pri

	DEF_SNIPPET(0x38);
	emit zero.u global_var;			// zero 00000000
	emit zero.u local_static_var;	// zero 0000000c
	emit zero.u local_var;			// zero.s fffffffc
	emit zero.u local_refvar;		// zero.pri \ sref.s.pri 0000000c
	DEF_SNIPPET(0x39);
	emit zero.u global_array[0];		// zero 00000004
	emit zero.u global_array[1];		// zero 00000008
	emit zero.u local_static_array[0];	// zero 00000010
	emit zero.u local_static_array[1];	// zero 00000014
	emit zero.u local_array[0];			// zero.s fffffff4
	emit zero.u local_array[1];			// zero.s fffffff8
	emit zero.u local_refarray[0];		// zero.pri \ sref.s.pri 00000010
	emit zero.u local_refarray[1];		// load.s.pri 00000010 \ add.c 00000004 \ move.alt \ zero.pri \ stor.i
	DEF_SNIPPET(0x3a);
	emit zero.u global_array{0};		// const.alt 00000004 \ align.alt 00000001 \ zero.pri \ strb.i 00000001
	emit zero.u global_array{1};		// const.alt 00000005 \ align.alt 00000001 \ zero.pri \ strb.i 00000001
	emit zero.u local_static_array{0};	// const.alt 00000010 \ align.alt 00000001 \ zero.pri \ strb.i 00000001
	emit zero.u local_static_array{1};	// const.alt 00000011 \ align.alt 00000001 \ zero.pri \ strb.i 00000001
	emit zero.u local_array{0};			// addr.alt fffffff4 \ align.alt 00000001 \ zero.pri \ strb.i 00000001
	emit zero.u local_array{1};			// addr.alt fffffff5 \ align.alt 00000001 \ zero.pri \ strb.i 00000001
	emit zero.u local_refarray{0};		// load.s.alt 00000010 \ align.alt 00000001 \ zero.pri \ strb.i 00000001
	emit zero.u local_refarray{1};		// load.s.alt 00000010 \ inc.alt \ align.alt 00000001 \ zero.pri \ strb.i 00000001
	emit zero.u local_refarray{2};		// load.s.pri 00000010 \ add.c 00000002 \ move.alt \ align.alt 00000001 \ zero.pri \ strb.i 00000001
	DEF_SNIPPET(0x3b);
	emit zero.u global_array[local_var];		// load.s.pri fffffffc \ const.alt 00000004 \ idxaddr \ move.alt \ zero.pri \ stor.i
	emit zero.u local_static_array[local_var];	// load.s.pri fffffffc \ const.alt 00000010 \ idxaddr \ move.alt \ zero.pri \ stor.i
	emit zero.u local_array[local_var];			// load.s.pri fffffffc \ addr.alt fffffff4 \ idxaddr \ move.alt \ zero.pri \ stor.i
	emit zero.u local_refarray[local_var];		// load.s.pri fffffffc \ load.s.alt 00000010 \ idxaddr \ move.alt \ zero.pri \ stor.i
	DEF_SNIPPET(0x3c);
	emit zero.u global_array{local_var};		// load.s.pri fffffffc \ add.c 00000004 \ align.pri 00000001 \ move.alt \ zero.pri \ strb.i 00000001
	emit zero.u local_static_array{local_var};	// load.s.pri fffffffc \ add.c 00000010 \ align.pri 00000001 \ move.alt \ zero.pri \ strb.i 00000001
	emit zero.u local_array{local_var};			// load.s.pri fffffffc \ addr.alt fffffff4 \ add \ align.pri 00000001 \ move.alt \ zero.pri \ strb.i 00000001
	emit zero.u local_refarray{local_var};		// load.s.pri fffffffc \ load.s.alt 00000010 \ add \ align.pri 00000001 \ move.alt \ zero.pri \ strb.i 00000001

	DEF_SNIPPET(0x40);
	emit inc.u global_var;			// inc 00000000
	emit inc.u local_static_var;	// inc 0000000c
	emit inc.u local_var;			// inc.s fffffffc
	emit inc.u local_refvar;		// load.s.pri 0000000c \ inc.i
	DEF_SNIPPET(0x41);
	emit inc.u global_array[0];			// inc 00000004
	emit inc.u global_array[1];			// inc 00000008
	emit inc.u local_static_array[0];	// inc 00000010
	emit inc.u local_static_array[1];	// inc 00000014
	emit inc.u local_array[0];			// inc.s fffffff4
	emit inc.u local_array[1];			// inc.s fffffff8
	emit inc.u local_refarray[0];		// load.s.pri 00000010 \ inc.i
	emit inc.u local_refarray[1];		// load.s.pri 00000010 \ add.c 00000004 \ inc.i
	DEF_SNIPPET(0x42);
	emit inc.u global_array{0};			// const.pri 00000004 \ align.pri 00000001 \ move.alt \ lodb.i 00000001 \ inc.pri \ strb.i 00000001
	emit inc.u global_array{1};			// const.pri 00000005 \ align.pri 00000001 \ move.alt \ lodb.i 00000001 \ inc.pri \ strb.i 00000001
	emit inc.u local_static_array{0};	// const.pri 00000010 \ align.pri 00000001 \ move.alt \ lodb.i 00000001 \ inc.pri \ strb.i 00000001
	emit inc.u local_static_array{1};	// const.pri 00000011 \ align.pri 00000001 \ move.alt \ lodb.i 00000001 \ inc.pri \ strb.i 00000001
	emit inc.u local_array{0};			// addr.pri fffffff4 \ align.pri 00000001 \ move.alt \ lodb.i 00000001 \ inc.pri \ strb.i 00000001
	emit inc.u local_array{1};			// addr.pri fffffff5 \ align.pri 00000001 \ move.alt \ lodb.i 00000001 \ inc.pri \ strb.i 00000001
	emit inc.u local_refarray{0};		// load.s.pri 00000010 \ align.pri 00000001 \ move.alt \ lodb.i 00000001 \ inc.pri \ strb.i 00000001
	emit inc.u local_refarray{1};		// load.s.pri 00000010 \ inc.pri \ align.pri 00000001 \ move.alt \ lodb.i 00000001 \ inc.pri \ strb.i 00000001
	emit inc.u local_refarray{2};		// load.s.pri 00000010 \ add.c 00000002 \ align.pri 00000001 \ move.alt \ lodb.i 00000001 \ inc.pri \ strb.i 00000001
	DEF_SNIPPET(0x43);
	emit inc.u global_array[local_var];			// load.s.pri fffffffc \ const.alt 00000004 \ idxaddr \ inc.i
	emit inc.u local_static_array[local_var];	// load.s.pri fffffffc \ const.alt 00000010 \ idxaddr \ inc.i
	emit inc.u local_array[local_var];			// load.s.pri fffffffc \ addr.alt fffffff4 \ idxaddr \ inc.i
	emit inc.u local_refarray[local_var];		// load.s.pri fffffffc \ load.s.alt 00000010 \ idxaddr \ inc.i
	DEF_SNIPPET(0x44);
	emit inc.u global_array{local_var};			// load.s.pri fffffffc \ add.c 00000004 \ align.pri 00000001 \ move.alt \ lodb.i 00000001 \ inc.pri \ strb.i 00000001
	emit inc.u local_static_array{local_var};	// load.s.pri fffffffc \ add.c 00000010 \ align.pri 00000001 \ move.alt \ lodb.i 00000001 \ inc.pri \ strb.i 00000001
	emit inc.u local_array{local_var};			// load.s.pri fffffffc \ addr.alt fffffff4 \ add \ align.pri 00000001 \ move.alt \ lodb.i 00000001 \ inc.pri \ strb.i 00000001
	emit inc.u local_refarray{local_var};		// load.s.pri fffffffc \ load.s.alt 00000010 \ add \ align.pri 00000001 \ move.alt \ lodb.i 00000001 \ inc.pri \ strb.i 00000001

	emit nop; // end marker
}


main()
{
	new t, a[2];
	test__pcode(t, a);
}
