/* LCM type definition class file
 * This file was automatically generated by lcm-gen
 * DO NOT MODIFY BY HAND!!!!
 */

package edu.umich.robot.lcmtypes;
 
import java.io.*;
import java.nio.*;
import java.util.*;
import lcm.lcm.*;
 
public final class map_metadata_t implements lcm.lcm.LCMEncodable
{
    public long utime;
    public int nareas;
    public int area_ids[];
    public double areas[][];
    public int ngateways;
    public int gateway_ids[];
    public double gateways[][];
 
    public map_metadata_t()
    {
    }
 
    public static final long LCM_FINGERPRINT;
    public static final long LCM_FINGERPRINT_BASE = 0x9205a981bbfc735eL;
 
    static {
        LCM_FINGERPRINT = _hashRecursive(new ArrayList<Class>());
    }
 
    public static long _hashRecursive(ArrayList<Class> classes)
    {
        if (classes.contains(map_metadata_t.class))
            return 0L;
 
        classes.add(map_metadata_t.class);
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
        outs.writeLong(this.utime); 
 
        outs.writeInt(this.nareas); 
 
        for (int a = 0; a < this.nareas; a++) {
            outs.writeInt(this.area_ids[a]); 
        }
 
        for (int a = 0; a < this.nareas; a++) {
            for (int b = 0; b < 4; b++) {
                outs.writeDouble(this.areas[a][b]); 
            }
        }
 
        outs.writeInt(this.ngateways); 
 
        for (int a = 0; a < this.ngateways; a++) {
            outs.writeInt(this.gateway_ids[a]); 
        }
 
        for (int a = 0; a < this.ngateways; a++) {
            for (int b = 0; b < 2; b++) {
                outs.writeDouble(this.gateways[a][b]); 
            }
        }
 
    }
 
    public map_metadata_t(byte[] data) throws IOException
    {
        this(new LCMDataInputStream(data));
    }
 
    public map_metadata_t(DataInput ins) throws IOException
    {
        if (ins.readLong() != LCM_FINGERPRINT)
            throw new IOException("LCM Decode error: bad fingerprint");
 
        _decodeRecursive(ins);
    }
 
    public static map_metadata_t _decodeRecursiveFactory(DataInput ins) throws IOException
    {
        map_metadata_t o = new map_metadata_t();
        o._decodeRecursive(ins);
        return o;
    }
 
    public void _decodeRecursive(DataInput ins) throws IOException
    {
        this.utime = ins.readLong();
 
        this.nareas = ins.readInt();
 
        this.area_ids = new int[(int) nareas];
        for (int a = 0; a < this.nareas; a++) {
            this.area_ids[a] = ins.readInt();
        }
 
        this.areas = new double[(int) nareas][(int) 4];
        for (int a = 0; a < this.nareas; a++) {
            for (int b = 0; b < 4; b++) {
                this.areas[a][b] = ins.readDouble();
            }
        }
 
        this.ngateways = ins.readInt();
 
        this.gateway_ids = new int[(int) ngateways];
        for (int a = 0; a < this.ngateways; a++) {
            this.gateway_ids[a] = ins.readInt();
        }
 
        this.gateways = new double[(int) ngateways][(int) 2];
        for (int a = 0; a < this.ngateways; a++) {
            for (int b = 0; b < 2; b++) {
                this.gateways[a][b] = ins.readDouble();
            }
        }
 
    }
 
    public map_metadata_t copy()
    {
        map_metadata_t outobj = new map_metadata_t();
        outobj.utime = this.utime;
 
        outobj.nareas = this.nareas;
 
        outobj.area_ids = new int[(int) nareas];
        if (this.nareas > 0)
            System.arraycopy(this.area_ids, 0, outobj.area_ids, 0, this.nareas); 
        outobj.areas = new double[(int) nareas][(int) 4];
        for (int a = 0; a < this.nareas; a++) {
            System.arraycopy(this.areas[a], 0, outobj.areas[a], 0, 4);        }
 
        outobj.ngateways = this.ngateways;
 
        outobj.gateway_ids = new int[(int) ngateways];
        if (this.ngateways > 0)
            System.arraycopy(this.gateway_ids, 0, outobj.gateway_ids, 0, this.ngateways); 
        outobj.gateways = new double[(int) ngateways][(int) 2];
        for (int a = 0; a < this.ngateways; a++) {
            System.arraycopy(this.gateways[a], 0, outobj.gateways[a], 0, 2);        }
 
        return outobj;
    }
 
}

