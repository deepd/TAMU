package utilities;

import java.lang.reflect.Array;
import java.util.HashSet;
import java.util.Iterator;

/**
 * Created by deep on 11/29/14.
 */
public class SET<Key extends Comparable<Key>> implements Iterable<Key> {
    private HashSet<Key> set;

    public SET() {
        set = new HashSet<Key>(5000);
    }

    public void add(Key key) {
        if (key == null) throw new NullPointerException("called add() with a null key");
        set.add(key);
    }

    public boolean contains(Key key) {
        if (key == null) throw new NullPointerException("called contains() with a null key");
        return set.contains(key);
    }

    public void delete(Key key) {
        if (key == null) throw new NullPointerException("called delete() with a null key");
        set.remove(key);
    }

    public int size() {
        return set.size();
    }

    public boolean isEmpty() {
        return size() == 0;
    }

    public Iterator<Key> iterator() {
        return set.iterator();
    }

    public boolean equals(Object y) {
        if (y == this) return true;
        if (y == null) return false;
        if (y.getClass() != this.getClass()) return false;
        SET<Key> that = (SET<Key>) y;
        if (this.size() != that.size()) return false;
        try {
            for (Key k : this)
                if (!that.contains(k)) return false;
        }
        catch (ClassCastException exception) {
            return false;
        }
        return true;
    }

    public String toString() {
        StringBuilder s = new StringBuilder();
        for (Key key : this)
            s.append(key + " ");
        return s.toString();
    }

    public <T> T[] toArray(Class<T> type, T[] a) {
        int size = size();
        T[] array;
        array = (T[]) Array.newInstance(type, size);
        int i = 0;
        for (Key k: this) {
            array[i] = (T) k;
            i++;
        }
        return array;
    }

}