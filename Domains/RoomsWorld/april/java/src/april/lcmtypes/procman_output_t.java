/* LCM type definition class file
 * This file was automatically generated by lcm-gen
 * DO NOT MODIFY BY HAND!!!!
 */

package april.lcmtypes;
 
import java.io.*;
import java.nio.*;
import java.util.*;
import lcm.lcm.*;
 
public final class procman_output_t implements lcm.lcm.LCMEncodable
{
    public long utime;
    public String host;
    public int procid;
    public int stream;
    public String data;
 
    public procman_output_t()
    {
    }
 
    public static final long LCM_FINGERPRINT;
    public static final long LCM_FINGERPRINT_BASE = 0x6fda1e123c77d5e9L;
 
    static {
        LCM_FINGERPRINT = _hashRecursive(new ArrayList<Class>());
    }
 
    public static long _hashRecursive(ArrayList<Class> classes)
    {
        if (classes.contains(april.lcmtypes.procman_output_t.class))
            return 0L;
 
        classes.add(april.lcmtypes.procman_output_t.class);
        long hash = LCM_FINGERPRINT_BASE
            ;
        classes.remove(classes.size() - 1);
        return (hash<<1) + ((hash>>63)&1);
    }
 
    public void encode(DataOutput outs) throws IOException
    {
        outs.writeLong(LCM_FINGERPRINT);
        _encodeRecursive(outs);
    }
 
    public void _encodeRecursive(DataOutput outs) throws IOException
    {
        char[] __strbuf = null;
        outs.writeLong(this.utime); 
 
        __strbuf = new char[this.host.length()]; this.host.getChars(0, this.host.length(), __strbuf, 0); outs.writeInt(__strbuf.length+1); for (int _i = 0; _i < __strbuf.length; _i++) outs.write(__strbuf[_i]); outs.writeByte(0); 
 
        outs.writeInt(this.procid); 
 
        outs.writeInt(this.stream); 
 
        __strbuf = new char[this.data.length()]; this.data.getChars(0, this.data.length(), __strbuf, 0); outs.writeInt(__strbuf.length+1); for (int _i = 0; _i < __strbuf.length; _i++) outs.write(__strbuf[_i]); outs.writeByte(0); 
 
    }
 
    public procman_output_t(byte[] data) throws IOException
    {
        this(new LCMDataInputStream(data));
    }
 
    public procman_output_t(DataInput ins) throws IOException
    {
        if (ins.readLong() != LCM_FINGERPRINT)
            throw new IOException("LCM Decode error: bad fingerprint");
 
        _decodeRecursive(ins);
    }
 
    public static april.lcmtypes.procman_output_t _decodeRecursiveFactory(DataInput ins) throws IOException
    {
        april.lcmtypes.procman_output_t o = new april.lcmtypes.procman_output_t();
        o._decodeRecursive(ins);
        return o;
    }
 
    public void _decodeRecursive(DataInput ins) throws IOException
    {
        char[] __strbuf = null;
        this.utime = ins.readLong();
 
        __strbuf = new char[ins.readInt()-1]; for (int _i = 0; _i < __strbuf.length; _i++) __strbuf[_i] = (char) (ins.readByte()&0xff); ins.readByte(); this.host = new String(__strbuf);
 
        this.procid = ins.readInt();
 
        this.stream = ins.readInt();
 
        __strbuf = new char[ins.readInt()-1]; for (int _i = 0; _i < __strbuf.length; _i++) __strbuf[_i] = (char) (ins.readByte()&0xff); ins.readByte(); this.data = new String(__strbuf);
 
    }
 
    public april.lcmtypes.procman_output_t copy()
    {
        april.lcmtypes.procman_output_t outobj = new april.lcmtypes.procman_output_t();
        outobj.utime = this.utime;
 
        outobj.host = this.host;
 
        outobj.procid = this.procid;
 
        outobj.stream = this.stream;
 
        outobj.data = this.data;
 
        return outobj;
    }
 
}

