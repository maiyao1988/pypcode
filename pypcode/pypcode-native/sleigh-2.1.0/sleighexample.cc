/* ###
 * IP: GHIDRA
 * NOTE: mentions GNU libbfd, the hard-coded binary is a toy function that generates primes
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
// Dump the raw pcode instructions

// Root include for parsing using SLEIGH
#include <loadimage.hh>
#include <sleigh.hh>
#include <emulate.hh>
#include <iostream>
#include <llvm/IR/IRBuilder.h>

// These are the bytes for an example x86 binary
// These bytes are loaded at address 0x80483b4
static uint1 myprog[] = {
	0x8d, 0x4c, 0x24, 0x04, 0x83, 0xe4, 0xf0, 0xff, 0x71, 0xfc, 0x55,
	0x89, 0xe5, 0x51, 0x81, 0xec, 0xb4, 0x01, 0x00, 0x00, 0xc7, 0x45, 0xf4,
	0x00, 0x00, 0x00, 0x00, 0xeb, 0x12, 0x8b, 0x45, 0xf4, 0xc7, 0x84,
	0x85, 0x64, 0xfe, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x83, 0x45, 0xf4,
	0x01, 0x83, 0x7d, 0xf4, 0x63, 0x7e, 0xe8, 0xc7, 0x45, 0xf4, 0x02,
	0x00, 0x00, 0x00, 0xeb, 0x28, 0x8b, 0x45, 0xf4, 0x01, 0xc0, 0x89, 0x45,
	0xf8, 0xeb, 0x14, 0x8b, 0x45, 0xf8, 0xc7, 0x84, 0x85, 0x64, 0xfe,
	0xff, 0xff, 0x01, 0x00, 0x00, 0x00, 0x8b, 0x45, 0xf4, 0x01, 0x45, 0xf8,
	0x83, 0x7d, 0xf8, 0x63, 0x7e, 0xe6, 0x83, 0x45, 0xf4, 0x01, 0x83,
	0x7d, 0xf4, 0x31, 0x7e, 0xd2, 0xc7, 0x04, 0x24, 0x40, 0x85, 0x04, 0x08,
	0xe8, 0x9c, 0xfe, 0xff, 0xff, 0xc7, 0x45, 0xf4, 0x02, 0x00, 0x00,
	0x00, 0xeb, 0x25, 0x8b, 0x45, 0xf4, 0x8b, 0x84, 0x85, 0x64, 0xfe, 0xff,
	0xff, 0x85, 0xc0, 0x75, 0x13, 0x8b, 0x45, 0xf4, 0x89, 0x44, 0x24,
	0x04, 0xc7, 0x04, 0x24, 0x47, 0x85, 0x04, 0x08, 0xe8, 0x62, 0xfe, 0xff,
	0xff, 0x83, 0x45, 0xf4, 0x01, 0x83, 0x7d, 0xf4, 0x63, 0x7e, 0xd5,
	0x81, 0xc4, 0xb4, 0x01, 0x00, 0x00, 0x59, 0x5d, 0x8d, 0x61, 0xfc, 0xc3,
	0x90, 0x90, 0x90, 0x90, 0x55, 0x89, 0xe5, 0x5d, 0xc3, 0x8d, 0x74,
	0x26, 0x00, 0x8d, 0xbc, 0x27, 0x00, 0x00, 0x00, 0x00, 0x55, 0x89, 0xe5,
	0x57, 0x56, 0x53, 0xe8, 0x5e, 0x00, 0x00, 0x00, 0x81, 0xc3, 0xa5,
	0x11, 0x00, 0x00, 0x83, 0xec, 0x1c, 0xe8, 0xd7, 0xfd, 0xff, 0xff, 0x8d,
	0x83, 0x20, 0xff, 0xff, 0xff, 0x89, 0x45, 0xf0, 0x8d, 0x83, 0x20,
	0xff, 0xff, 0xff, 0x29, 0x45, 0xf0, 0xc1, 0x7d, 0xf0, 0x02, 0x8b, 0x55,
	0xf0, 0x85, 0xd2, 0x74, 0x2b, 0x31, 0xff, 0x89, 0xc6, 0x8d, 0xb6,
	0x00, 0x00, 0x00, 0x00, 0x8b, 0x45, 0x10, 0x83, 0xc7, 0x01, 0x89, 0x44,
	0x24, 0x08, 0x8b, 0x45, 0x0c, 0x89, 0x44, 0x24, 0x04, 0x8b, 0x45,
	0x08, 0x89, 0x04, 0x24, 0xff, 0x16, 0x83, 0xc6, 0x04, 0x39, 0x7d, 0xf0,
	0x75, 0xdf, 0x83, 0xc4, 0x1c, 0x5b, 0x5e, 0x5f, 0x5d, 0xc3, 0x8b,
	0x1c, 0x24, 0xc3, 0x90, 0x90, 0x90, 0x55, 0x89, 0xe5, 0x53, 0xbb, 0x50,
	0x95, 0x04, 0x08, 0x83, 0xec, 0x04, 0xa1, 0x50, 0x95, 0x04, 0x08,
	0x83, 0xf8, 0xff, 0x74, 0x0c, 0x83, 0xeb, 0x04, 0xff, 0xd0, 0x8b, 0x03,
	0x83, 0xf8, 0xff, 0x75, 0xf4, 0x83, 0xc4, 0x04, 0x5b, 0x5d, 0xc3,
	0x55, 0x89, 0xe5, 0x53, 0x83, 0xec, 0x04, 0xe8, 0x00, 0x00, 0x00, 0x00,
	0x5b, 0x81, 0xc3, 0x0c, 0x11, 0x00, 0x00, 0xe8, 0x00, 0xfe, 0xff,
	0xff, 0x59, 0x5b, 0xc9, 0xc3, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x72, 0x69, 0x6d, 0x65, 0x73,
	0x00, 0x25, 0x64, 0x0a, 0x00, 0x00
};  // Size of 408 bytes

static uint1 myprog_arm[] = {0xf0,0xb5,0x03,0xaf,0x4d,0xf8,0x04,0xbd,0x82,0xb0,0x04,0x46,0x2d,0x48,0x78,0x44,0x06,0x68,0x30,0x68,0x01,0x90,0x00,0xf0,0x5b,0xf8,0x08,0xb1,0xff,0xf7,0x00,0xff,0x00,0xf0,0x84,0xf8,0x08,0xb1,0xff,0xf7,0x77,0xff,0xff,0xf7,0xf9,0xfe,0x08,0xb1,0x00,0xf0,0x4e,0xf8,0xff,0xf7,0x70,0xff,0x08,0xb1,0xff,0xf7,0xf1,0xfe,0xff,0xf7,0xef,0xfe,0x08,0xb1,0x00,0xf0,0x72,0xf8,0x00,0xf0,0x42,0xf8,0x08,0xb1,0xff,0xf7,0x63,0xff,0xff,0xf7,0x61,0xff,0x08,0xb1,0xff,0xf7,0xe2,0xfe,0xff,0xf7,0xe0,0xfe,0x08,0xb1,0x00,0xf0,0x63,0xf8,0x00,0x20,0x15,0x4d,0x00,0x90,0x69,0x46,0x20,0x68,0x2a,0x46,0x83,0x69,0x20,0x46,0x98,0x47,0x60,0xb1,0x4f,0xf0,0xff,0x35,0x01,0x98,0x31,0x68,0x08,0x1a,0x01,0xbf,0x28,0x46,0x02,0xb0,0x5d,0xf8,0x04,0xbb,0xf0,0xbd,0xfe,0xf7,0x8a,0xea,0x00,0x9c,0x20,0x68,0x0b,0x49,0x82,0x69,0x79,0x44,0x20,0x46,0x90,0x47,0x00,0x28,0xe9,0xd0,0x01,0x46,0x20,0x68,0x02,0x23,0xd0,0xf8,0x5c,0xc3,0x20,0x46,0x06,0x4a,0x7a,0x44,0xe0,0x47,0xb0,0xf1,0xff,0x3f,0xdf,0xdc,0xdc,0xe7};//size 192

// This is a tiny LoadImage class which feeds the executable bytes to the translator
class MyLoadImage : public LoadImage {
	uintb baseaddr;
	int4 length;
	uint1 *data;
	public:
	MyLoadImage(uintb ad,uint1 *ptr,int4 sz) : LoadImage("nofile") { baseaddr = ad; data = ptr; length = sz; }
	virtual void loadFill(uint1 *ptr,int4 size,const Address &addr);
	virtual string getArchType(void) const { return "myload"; }
	virtual void adjustVma(long adjust) { }
};

// This is the only important method for the LoadImage. It returns bytes from the static array
// depending on the address range requested
void MyLoadImage::loadFill(uint1 *ptr,int4 size,const Address &addr)

{
	uintb start = addr.getOffset();
	uintb max = baseaddr + (length-1);
	for(int4 i=0;i<size;++i) {	// For every byte requestes
		uintb curoff = start + i; // Calculate offset of byte
		if ((curoff < baseaddr)||(curoff>max)) {	// If byte does not fall in window
			ptr[i] = 0;		// return 0
			continue;
		}
		uintb diff = curoff - baseaddr;
		ptr[i] = data[(int4)diff];	// Otherwise return data from our window
	}
}

// -------------------------------
//
// These are the classes/routines relevant to doing disassembly 

// Here is a simple class for emitting assembly.  In this case, we send the strings straight
// to standard out.
class AssemblyRaw : public AssemblyEmit {
	public:
		virtual void dump(const Address &addr,const string &mnem,const string &body) {
			addr.printRaw(cout);
			cout << ": " << mnem << ' ' << body << endl;
		}
};

static void dumpAssembly(Translate &trans, ContextInternal &ctx, unsigned base_addr, size_t sz)

{ // Print disassembly of binary code
	AssemblyRaw assememit;	// Set up the disassembly dumper
	int4 length;			// Number of bytes of each machine instruction

	//Address addr(trans.getDefaultSpace(),0x80483b4); // First disassembly address
	//Address lastaddr(trans.getDefaultSpace(),0x804846c); // Last disassembly address

    Address addr(trans.getDefaultSpace(),base_addr); // First address to translate
    Address lastaddr(trans.getDefaultSpace(),base_addr+sz); // Last address

	while(addr < lastaddr) {
		length = trans.printAssembly(assememit,addr);
		addr = addr + length;
	}
}

// -------------------------------
//
// These are the classes/routines relevant to printing a pcode translation

// Here is a simple class for emitting pcode. We simply dump an appropriate string representation
// straight to standard out.
class PcodeRawOut : public PcodeEmit {
	public:
		virtual void dump(const Address &addr,OpCode opc,VarnodeData *outvar,VarnodeData *vars,int4 isize);
};

static void print_vardata(ostream &s,VarnodeData &data)

{
	s << '(' << data.space->getName() << ',';
	data.space->printOffset(s,data.offset);
	s << ',' << dec << data.size << ')';
}

void PcodeRawOut::dump(const Address &addr,OpCode opc,VarnodeData *outvar,VarnodeData *vars,int4 isize)

{
	if (outvar != (VarnodeData *)0) {
		print_vardata(cout,*outvar);
		cout << " = ";
	}
    
	cout << get_opname(opc);
	// Possibly check for a code reference or a space reference
	for(int4 i=0;i<isize;++i) {
		cout << ' ';
		print_vardata(cout,vars[i]);
	}
	cout << endl;
}

static void dumpPcode(Translate &trans, ContextInternal &ctx, unsigned base_addr, size_t sz, bool isArm)

{ // Dump pcode translation of machine instructions
	PcodeRawOut emit;		// Set up the pcode dumper
	AssemblyRaw assememit;	// Set up the disassembly dumper
	int4 length;			// Number of bytes of each machine instruction

    Address addr(trans.getDefaultSpace(),base_addr); // First address to translate

    Address lastaddr(trans.getDefaultSpace(),base_addr+sz); // Last address


	while(addr < lastaddr) {
		cout << "--- ";
		length = trans.printAssembly(assememit,addr);
		length = trans.oneInstruction(emit,addr); // Translate instruction
		addr = addr + length;		// Advance to next instruction
        if (isArm) {
            //实验知,oneInstruction调用后,blx下一条指令会自动将ISA_MODE改成0,也就是arm,这回导致thumb被解析成arm出错,所以每条指令结束需要设置解析器模式
            ctx.setVariable("ISA_MODE", addr, 1);
        }
        
	}
}

// -------------------------------------
//
// These are the classes/routines relevant for emulating the executable

// A simple class for emulating the system "puts" call.
// It justs looks up the string data and dumps it to standard out.
class PutsCallBack : public BreakCallBack {
	public:
		virtual bool addressCallback(const Address &addr);
};

bool PutsCallBack::addressCallback(const Address &addr)

{
	MemoryState *mem = static_cast<EmulateMemory *>(emulate)->getMemoryState();
	uint1 buffer[256];
	uint4 esp = mem->getValue("ESP");
	AddrSpace *ram = mem->getTranslate()->getSpaceByName("ram");

	uint4 param1 = mem->getValue(ram,esp+4,4);
	mem->getChunk(buffer,ram,param1,255);

	cout << (char *)&buffer << endl;

	uint4 returnaddr = mem->getValue(ram,esp,4);
	mem->setValue("ESP",esp+8);
	emulate->setExecuteAddress(Address(ram,returnaddr));

	return true;			// This replaces the indicated instruction
}

// A simple class for emulating the system "printf" call.
// We don't really emulate all of it.  The only printf call in the example
// has an initial string of "%d\n". So we grab the second parameter from the
// memory state and print it as an integer
class PrintfCallBack : public BreakCallBack {
	public:
		virtual bool addressCallback(const Address &addr);
};

bool PrintfCallBack::addressCallback(const Address &addr)

{
	MemoryState *mem = static_cast<EmulateMemory *>(emulate)->getMemoryState();

	AddrSpace *ram = mem->getTranslate()->getSpaceByName("ram");

	uint4 esp = mem->getValue("ESP");
	uint4 param2 = mem->getValue(ram,esp+8,4);
	cout << (int4)param2 << endl;

	uint4 returnaddr = mem->getValue(ram,esp,4);
	mem->setValue("ESP",esp+12);
	emulate->setExecuteAddress(Address(ram,returnaddr));

	return true;
}

// A callback that terminates the emulation
class TerminateCallBack : public BreakCallBack {
	public:
		virtual bool addressCallback(const Address &addr);
};

bool TerminateCallBack::addressCallback(const Address &addr)

{
	emulate->setHalt(true);

	return true;
}

static void doEmulation(Translate &trans,LoadImage &loader)

{
	// Set up memory state object
	MemoryImage loadmemory(trans.getDefaultSpace(),8,4096,&loader);
	MemoryPageOverlay ramstate(trans.getDefaultSpace(),8,4096,&loadmemory);
	MemoryHashOverlay registerstate(trans.getSpaceByName("register"),8,4096,4096,(MemoryBank *)0);
	MemoryHashOverlay tmpstate(trans.getUniqueSpace(),8,4096,4096,(MemoryBank *)0);

    MemoryState memstate(&trans);	// Instantiate the memory state object
	memstate.setMemoryBank(&ramstate);
	memstate.setMemoryBank(&registerstate);
	memstate.setMemoryBank(&tmpstate);

	BreakTableCallBack breaktable(&trans); // Set up the callback object
	EmulatePcodeCache emulater(&trans,&memstate,&breaktable); // Set up the emulator

	// Set up the initial register state for execution
	memstate.setValue("ESP",0xbffffffc);
	emulater.setExecuteAddress(Address(trans.getDefaultSpace(),0x80483b4));

	// Register callbacks
	PutsCallBack putscallback;
	PrintfCallBack printfcallback;
	TerminateCallBack terminatecallback;
	breaktable.registerAddressCallback(Address(trans.getDefaultSpace(),0x80482c8),&putscallback);
	breaktable.registerAddressCallback(Address(trans.getDefaultSpace(),0x80482b8),&printfcallback);
	breaktable.registerAddressCallback(Address(trans.getDefaultSpace(),0x804846b),&terminatecallback);

	emulater.setHalt(false);

	do {
		emulater.executeInstruction();
	} while(!emulater.getHalt());
}

int main(int argc,char **argv)

{
	if (argc != 2) {
		cerr << "USAGE:  " << argv[0] << " disassemble" << endl;
		cerr << "        " << argv[0] << " pcode" << endl;
		cerr << "        " << argv[0] << " emulate" << endl;
		return 2;
	}
	string action(argv[1]);
    
    bool isArm = true;
    unsigned base_addr = 0;
    uint1 *ins_buff = 0;
    size_t buff_sz = 0;
    string sleighfilename = "";
    if (isArm) {
        base_addr = 0x00008f90;
        ins_buff = myprog_arm;
        buff_sz = sizeof(myprog_arm);
        sleighfilename = "specfiles/ARM8_le.sla";
    }
    else {
        base_addr = 0x80483b4;
        ins_buff = myprog;
        buff_sz = sizeof(myprog);
        sleighfilename = "specfiles/x86.sla";
    }
    
	// Set up the loadimage
	MyLoadImage loader(base_addr,ins_buff,buff_sz);

	// Set up the context object
	ContextInternal context;

	// Set up the assembler/pcode-translator
	Sleigh trans(&loader,&context);

	// Read sleigh file into DOM
	DocumentStorage docstorage;
	Element *sleighroot = docstorage.openDocument(sleighfilename)->getRoot();
	docstorage.registerTag(sleighroot);
	trans.initialize(docstorage); // Initialize the translator
    
	// Now that context symbol names are loaded by the translator
	// we can set the default context
    if (isArm)
        context.setVariableDefault("ISA_MODE", 1);
    else {
        //x86
        context.setVariableDefault("addrsize",1); // Address size is 32-bit
        context.setVariableDefault("opsize",1); // Operand size is 32-bit
    }
    if (action == "disassemble")
		dumpAssembly(trans, context, base_addr, buff_sz);
	else if (action == "pcode")
		dumpPcode(trans, context, base_addr, buff_sz, isArm);
	else if (action == "emulate")
		doEmulation(trans,loader);
	else
		cerr << "Unknown action: "+action << endl;
}

