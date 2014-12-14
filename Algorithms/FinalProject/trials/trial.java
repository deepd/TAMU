package trials;

import utilities.SET;
import utilities.utils;



/**
 * Created by deep on 11/21/14.
 */
public class trial extends utils {
    public trial() {
        SET<Edge> s;
        s = new SET<Edge>();
        Edge e1 = new Edge(1,2,1);
        Edge e2 = new Edge(2,3,1);
        Edge e3 = new Edge(3,1,4);
        Edge e4 = new Edge(4,1,6);
        Edge e5 = new Edge(1,4,10);
        Edge e6 = new Edge(2,4,2);
        Edge e7 = new Edge(3,4,5);
        s.add(e1);
        s.add(e2);
        s.add(e3);
        s.add(e4);
        s.add(e5);
        s.add(e6);
        s.add(e7);
        Edge[] arr = s.toArray(Edge.class, new Edge[0]);
        for(int i = 0; i < arr.length; i++) {
            System.out.println(arr[i].either() + " -> " + arr[i].other(arr[i].either()) + " : " + arr[i].getWeight());
        }
        HeapSort<Edge> hs = new HeapSort<Edge>(arr);
        hs.sort();
        Edge[] arr2 = hs.sortedArray(Edge.class);
        int i = 0;
        /*for (Edge e : hs.sortedArray(Edge.class)) {
            arr2[i] = e;
            i++;
        }*/
        System.out.println("**********");

        for(Edge e : arr2) {
            System.out.println(e);
        }
        System.out.println("**********");
        MaxHeap mh = new MaxHeap(5);
        mh.insert(0,3);
        mh.insert(1,1);
        mh.insert(2,5);
        mh.insert(3,10);
        mh.insert(4,4);
        for(i = 0; i < 5; i++) {
            System.out.print(mh.maxIndex());
            System.out.println(" "+mh.maxValue());
            mh.delMax();
        }

    }

    public static void main (String []args) {
        trial t = new trial();
    }
}
