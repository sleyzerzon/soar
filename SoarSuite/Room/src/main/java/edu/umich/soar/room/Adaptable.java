package edu.umich.soar.room;

/**
 * Interface for something that can be adapted to another type of
 * object. See Eclipse's adapter framework
 * 
 * @author ray
 */
public interface Adaptable
{
    /**
     * Adapt this object to the given class or return null if not supported
     * 
     * @param klass The requested class or interface
     * @return An object of type klass or null if conversion is not supported.
     */
    Object getAdapter(Class<?> klass);
}
