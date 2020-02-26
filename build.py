import os
import codecs

outputfilename="cr8200"
compiler_command="g++ -o "
inclpaths=""
files=""

def addfolder(folder):
	global inclpaths
	global files
	inclpaths+="-I "
	inclpaths+=os.getcwd().replace("\\","/")
	inclpaths+="/"
	inclpaths+=folder
	inclpaths+=" "
	for file in os.listdir(folder):
		if file.endswith(".cpp") or file.endswith(".c"):
			files+=folder
			files+="/"
			files+=file
			files+=" "

def binaryimage(inputfile,outputfile):
	img=open(inputfile,"rb")
	header=open(outputfile,"w")
	xml=bytes("<IMAGE TYPE = \"JPEG\" WIDTH = \"1280\" HEIGHT = \"960\" />", "utf-8")
	header.write("const char testpicture[] = { ")
	bytecnt=0
	for char in xml:
		header.write("(char)")
		header.write(hex(char))
		header.write(", ")
		bytecnt=bytecnt+1
	byte=img.read(1)
	while byte != b"":
		if bytecnt%1008==0:
			header.write("\n")
		header.write("(char)0x")
		header.write(codecs.decode(codecs.encode(byte,"hex"),"utf-8"))
		header.write(", ")
		byte=img.read(1)
		bytecnt=bytecnt+1
	header.write("0x00 ")
	header.write("};\n")
	header.write("int testpicturesize = ")
	header.write(str(bytecnt+1))
	header.write(";");
	img.close()
	header.close()

# ZXing
addfolder("ZXing/core/src")
addfolder("ZXing/core/src/aztec")
addfolder("ZXing/core/src/datamatrix")
addfolder("ZXing/core/src/maxicode")
addfolder("ZXing/core/src/oned")
addfolder("ZXing/core/src/oned/rss")
addfolder("ZXing/core/src/pdf417")
addfolder("ZXing/core/src/qrcode")
addfolder("ZXing/core/src/textcodec")
addfolder("ZXing/thirdparty/stb")

# CRC
inclpaths+="-I "
inclpaths+=os.getcwd().replace("\\","/")
inclpaths+="/crc"
inclpaths+=" "
files+="crc.o "

# RS232
inclpaths+="-I "
inclpaths+=os.getcwd().replace("\\","/")
inclpaths+="/rs232"
inclpaths+=" "
files+="rs232.o "

# cr8200-sim
addfolder("scansim")
addfolder("scansim/handlers")


if os.name == "nt":
	outputfilename+=".exe"
	os.system( "gcc -c -o rs232.o rs232/rs232-win.c" )
	print( "OS Windows" )
else:
	os.system( "gcc -c -o rs232.o rs232/rs232-linux.c" )
	print( "OS Linux " + os.name )

os.system( "gcc -c -o crc.o crc/crc.c" )

binaryimage("scansim/handlers/testpicture.jpg","scansim/handlers/testpicture.h")

compiler_command+=outputfilename
compiler_command+=" "
compiler_command+=inclpaths
compiler_command+=files

os.system(compiler_command)

os.remove("crc.o")
os.remove("rs232.o")
os.remove("scansim/handlers/testpicture.h")


