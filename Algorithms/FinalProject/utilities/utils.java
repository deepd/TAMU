/**
 * Created by deep on 11/18/14.
 */

package utilities;

import java.lang.reflect.Array;
import java.util.*;


public class utils {
    public class Edge implements Comparable<Edge> {
        private final int v, w;
        private final int weight;
        public Edge(int v, int w, int weight) {
            this.v = v;
            this.w = w;
            this.weight = weight;
        }
        public int either() { return v;}
        public int other(int vertex) {
            if (vertex == v) return w;
            else return v;
        }

        public String toString() {
            return (v+"->"+w+":"+weight);
        }

        public int compareTo(Edge that) {
            if (this.weight < that.weight) return 1;
            else if (this.weight > that.weight) return -1;
            else return 1;
        }
        public int getWeight() {
            return weight;
        }
    }

    public class EdgeWeightedGraph {
        private int V;
        public  int M;
        private final SET<Edge>[] adj;
        private int[] degree;
        public SET<Edge> edges;

        public EdgeWeightedGraph(int V) {
            this.V = V;
            adj = (SET<Edge>[])new SET[V];
            edges = new SET<Edge>();
            degree = new int[V];
            M = 0;
            for (int v = 0; v < V; v++)
                adj[v] = new SET<Edge>();
        }

        public SET<Edge> edges() {
            return edges;
        }

        public int numberOfEdges() {
            return M;
        }

        public void addEdge(Edge e) {
            int v = e.either(), w = e.other(v);
            adj[v].add(e);
            adj[w].add(e);
            edges.add(e);
            M++;

        }

        public Iterable<Edge> adj(int v) {
            return adj[v];
        }

        private void makeSparseGraph() {
            for(int i = 0; i < V; i++) {
                int[] arrlist = new int[5000];
                int n = 0;
                for (int j = 0; j < V; j++) {
                    if (degree[j] < 6 && j != i) {
                        arrlist[n] = j;
                        n++;
                    }
                }
                int left = 6 - degree[i];
                for (int k = 0; k < n && left > 0; left--, k++) {
                    Random randG = new Random();
                    int rand = randG.nextInt(n-k);
                    int temp1 = arrlist[rand];
                    int temp2 = arrlist[n-k-1];
                    arrlist[n-k-1] = temp1;
                    arrlist[rand] = temp2;
                    int w = arrlist[n-k-1];
                    int wt =randG.nextInt(10000);
                    Edge e1 = new Edge(i, w ,wt);
                    addEdge(e1);
                    degree[i]++;
                    degree[w]++;
                }
            }
        }

        private void makeDenseGraph() {
            for(int i = 0; i < V; i++) {
                int[] arrlist = new int[5000];
                int n = 0;
                for (int j = 0; j < V; j++) {
                    if (degree[j] < 1000 && j != i) {
                        arrlist[n] = j;
                        n++;
                    }
                }
                int left = 1000 - degree[i];
                for (int k = 0; k < n && left > 0; left--, k++) {
                    Random randG = new Random();
                    int rand = randG.nextInt(n-k);
                    int temp1 = arrlist[rand];
                    int temp2 = arrlist[n-k-1];
                    arrlist[n-k-1] = temp1;
                    arrlist[rand] = temp2;
                    int w = arrlist[n-k-1];
                    int wt =randG.nextInt(10000);
                    Edge e1 = new Edge(i, w ,wt);
                    addEdge(e1);
                    degree[i]++;
                    degree[w]++;
                }
            }
        }

        public void addFullPath(int s, int t) {
            int arr[] = new int[5000];
            for(int i = 0; i < 5000; i++) {
                arr[i] = i;
            }
            int temp = arr[s];
            arr[s] = arr[0];
            arr[0] = temp;
            temp = arr[t];
            arr[t] = arr[4999];
            arr[4999] = temp;
            Random rnd = new Random();
            for (int i = arr.length - 2; i > 0; i--)
            {
                int index = rnd.nextInt(4999);
                if (index == 0)
                    continue;
                int a = arr[index];
                arr[index] = arr[i];
                arr[i] = a;
            }
            int flag = 0;
            for(int i = 0; i < 4999; i++) {
                for(Edge e: adj(arr[i])) {
                    if(e.other(arr[i]) == arr[i+1] || e.either() == arr[i+1])
                        flag = 1;
                }
                if (flag == 0) {
                    Edge e = new Edge(arr[i], arr[i + 1], rnd.nextInt(10000));
                    addEdge(e);
                }
            }
        }
    }


    private void DoStuffForSparse(EdgeWeightedGraph GSparse) {
        Random randG = new Random();
        for(int i = 0; i < 5; i++) {
            int s = randG.nextInt(5000);
            int t = randG.nextInt(5000);
            System.out.println("For " + s + " to " + t);
            GSparse.addFullPath(s, t);
            if(true) {
                long startTime = System.nanoTime();
                MaxBandwidthKruskal da = new MaxBandwidthKruskal(GSparse);
                long endTime = System.nanoTime();
                long duration = (endTime - startTime) / 1000000;
                da.makeTree();
                System.out.println("\t\tKRUSKAL :- Time: " + duration + "ms" +  " Bandwidth: " + da.bandwidth(s,t));
            }
            if(true) {
                long startTime = System.nanoTime();
                MaxBandwidthPathHeapDijkstras da = new MaxBandwidthPathHeapDijkstras(GSparse, s);
                long endTime = System.nanoTime();
                long duration = (endTime - startTime) / 1000000;
                System.out.println("\t\tDIJKSTRAS-HEAP :- Time: " + duration + "ms" +  " Bandwidth: " + da.bandwidth(t));
            }
            if(true) {
                long startTime = System.nanoTime();
                MaxBandwidthPathArrayDijkstras da = new MaxBandwidthPathArrayDijkstras(GSparse, s);
                long endTime = System.nanoTime();
                long duration = (endTime - startTime) / 1000000;
                System.out.println("\t\tDIJKSTRAS-ARRAY :- Time: " + duration + "ms" +  " Bandwidth: " + da.bandwidth(t));
            }
        }

    }

    private void DoStuffForDense(EdgeWeightedGraph GDense) {
        Random randG = new Random();
        for(int i = 0; i < 5; i++) {
            int s = randG.nextInt(5000);
            int t = randG.nextInt(5000);
            System.out.println("For " + s + " to " + t);
            GDense.addFullPath(s, t);
            if(true) {
                long startTime = System.nanoTime();
                MaxBandwidthPathHeapDijkstras da = new MaxBandwidthPathHeapDijkstras(GDense, s);
                long endTime = System.nanoTime();
                long duration = (endTime - startTime) / 1000000;
                System.out.println("\t\tDIJKSTRAS-HEAP :- Time: " + duration + "ms" +  " Bandwidth: " + da.bandwidth(t));
            }
            if(true) {
                long startTime = System.nanoTime();
                MaxBandwidthPathArrayDijkstras da = new MaxBandwidthPathArrayDijkstras(GDense, s);
                long endTime = System.nanoTime();
                long duration = (endTime - startTime) / 1000000;
                System.out.println("\t\tDIJKSTRAS-ARRAY :- Time: " + duration + "ms" +  " Bandwidth: " + da.bandwidth(t));
            }
            if(true) {
                long startTime = System.nanoTime();
                MaxBandwidthKruskal da = new MaxBandwidthKruskal(GDense);
                long endTime = System.nanoTime();
                long duration = (endTime - startTime) / 1000000;
                da.makeTree();
                System.out.println("\t\tKRUSKAL :- Time: " + duration + "ms" +  " Bandwidth: " + da.bandwidth(s,t));
            }
        }

    }

    public class GraphGenerator {
        public EdgeWeightedGraph GSparse, GDense;
        public GraphGenerator(int n) {
            for(int i =  1; i <= 5; i++) {
                System.out.println("Making Sparse Graph : " + i);
                System.out.println("***********************");
                GSparse = new EdgeWeightedGraph(n);
                GSparse.makeSparseGraph();
                DoStuffForSparse(GSparse);
            }
            for(int i =  1; i <= 5; i++) {
                System.out.println("Making Dense Graph : " + i);
                System.out.println("***********************");
                GDense = new EdgeWeightedGraph(n);
                GDense.makeDenseGraph();
                DoStuffForDense(GDense);
            }
        }
    }


    public class MaxBandwidthPathArrayDijkstras {
        private int[] d;
        private int[] status;
        private int fringes;
        private int[] dad;
        private int S;

        public MaxBandwidthPathArrayDijkstras(EdgeWeightedGraph G, int s) {
            S = s;
            status = new int[G.V];
            d = new int[G.V];
            fringes = 0;
            dad = new int[G.V];
            for (int i = 0; i < G.V; i++) {
                status[i] = -1;
                d[i] = -1;
            }
            status[S] = 1;
            d[S] = 0;
            dad[S] = -1;
            for (Edge e : G.adj(S)) {
                int v = e.other(S);
                status[v] = 0;
                fringes++;
                dad[v] = S;
                d[v] = e.weight;
            }
            while(fringes > 0) {
                int max = 0, v = 0;
                for(int i = 0; i < G.V; i++) {
                    if (status[i] == 0 && max < d[i]) {
                        max = d[i];
                        v = i;
                    }
                }
                status[v] = 1;
                fringes--;
                for (Edge e : G.adj(v)) {
                    int vd = e.other(v);
                    if(status[vd] == -1) {
                        status[vd] = 0;
                        fringes++;
                        dad[vd] = v;
                        d[vd] = Math.min(d[v],e.weight);
                    }
                    else if (status[vd] == 0 && d[vd] < Math.min(d[v],e.weight)) {
                        dad[vd] = v;
                        d[vd] = Math.min(d[v],e.weight);
                    }
                }
            }
        }

        public void pathTo(int v) {
            for(int i = v; i != -1; i = dad[i]) {
                System.out.println(i);
            }
        }

        public int bandwidth(int t) {
            return d[t];
        }
    }

    public class MaxBandwidthPathHeapDijkstras {
        private int[] d;
        private int[] status;
        private int[] dad;
        private int S;
        private MaxHeap fringeheap;

        public MaxBandwidthPathHeapDijkstras(EdgeWeightedGraph G, int s) {
            S = s;
            status = new int[G.V];
            d = new int[G.V];
            dad = new int[G.V];
            fringeheap = new MaxHeap(5000);
            for (int i = 0; i < G.V; i++) {
                status[i] = -1;
                d[i] = -1;
            }
            status[S] = 1;
            d[S] = 0;
            dad[S] = -1;
            for (Edge e : G.adj(S)) {
                int v = e.other(S);
                status[v] = 0;
                dad[v] = S;
                d[v] = e.weight;
                fringeheap.insert(v, d[v]);
            }
            while(!fringeheap.isEmpty()) {
                int v = 0;
                v = fringeheap.maxIndex();
                status[v] = 1;
                fringeheap.delMax();
                for (Edge e : G.adj(v)) {
                    int vd = e.other(v);
                    if(status[vd] == -1) {
                        status[vd] = 0;
                        dad[vd] = v;
                        d[vd] = Math.min(d[v],e.weight);
                        fringeheap.insert(vd, d[vd]);
                    }
                    else if (status[vd] == 0 && d[vd] < Math.min(d[v],e.weight)) {
                        dad[vd] = v;
                        d[vd] = Math.min(d[v],e.weight);
                        fringeheap.update(vd,d[vd]);
                    }
                }
            }
        }

        public void pathTo(int v) {
            for(int i = v; i != -1; i = dad[i]) {
                System.out.println(i);
            }
        }

        public int bandwidth(int v) {
            return d[v];
        }
    }

    public class UnionFind {
        private int []id, sz;
        private Stack<Integer> st;
        public UnionFind(int n) {
            id = new int[n];
            sz = new int[n];
            st = new Stack<Integer>();
            for (int i = 0; i < n; i++) {
                id[i] = i;
                sz[i] = 0;
            }
        }
        public boolean connected(int p, int q) {
            return find(p) == find(q);
        }
        public int find(int i) {
            while (i != id[i]) {
                st.push(i);
                i = id[i];
            }
            while (!st.isEmpty()) {
                int w = st.pop();
                id[w] = i;
            }
            return i;
        }
        public void union(int i, int j) {
            if (sz[i] < sz[j]) { id[i] = j; sz[j]+=sz[i]; }
            else	{ id[j] = i; sz[i]+=sz[j]; }
        }
    }

    public class HeapSort<Key extends Comparable<Key>> {
        private Key[] arr;
        private int N;
        public HeapSort(Key[] x) {
            arr = (Key[]) new Comparable[x.length + 1];
            for (int i = 0; i < x.length; i++) { arr[i+1] = x[i]; }
            N = arr.length - 1;
        }
        public void sort() {
            for(int i = N/2; i >= 1; i--) {
                sink(i);
            }
            int tmp = N;
            for(int i = 1; i <= tmp; i++) {
                exch(1,N--);
                sink(1);
            }
        }
        private void swim(int k) {
            while (k > 1 &&  less(k/2, k)) {
                exch(k, k/2);
                k = k/2;
            }
        }
        private void sink(int k) {
            while (2*k < N) {
                int j = 2*k;
                if (j < N){
                    boolean ans = less(j, j+1);
                    if(ans)
                        j++;
                }
                if (!less(k, j)) break;
                exch(j, k);
                k = j;
            }
        }
        private boolean less(int i, int j) {
            return arr[i].compareTo(arr[j]) < 0;
        }

        private void exch(int i, int j) {
            Key swap;
            swap = arr[i];
            arr[i] = arr[j];
            arr[j] = swap;
        }

        public <T> T[] sortedArray(Class<T> type) {
            T[] array;
            array = (T[]) Array.newInstance(type, arr.length - 1);
            for (int i = 0; i < arr.length - 1; i++) {
                array[i] = (T) arr[i+1];
            }
            return array;
        }

    }

    public class MaxBandwidthKruskal {
        private HashSet<Edge> T;
        private EdgeWeightedGraph Tree;
        private int[] bw;
        private boolean[] marked;
        private int[] edgeTo;
        private UnionFind uf;

        public MaxBandwidthKruskal(EdgeWeightedGraph G) {
            marked = new boolean[5000];
            edgeTo = new int[5000];
            T = new HashSet<Edge>(5000);
            Tree = new EdgeWeightedGraph(5000);
            bw = new int[5000];
            int ne = G.numberOfEdges(), counter = 0;
            SET<Edge> s = G.edges();
            Edge[] arr = s.toArray(Edge.class, new Edge[0]);
            HeapSort<Edge> hs;
            hs = new HeapSort<Edge>(arr);
            hs.sort();
            Edge[] arr2 = hs.sortedArray(Edge.class);
            uf = new UnionFind(5000);
            for (Edge e : arr2) {
                int v = e.either();
                int w = e.other(v);
                int r1 = uf.find(v);
                int r2 = uf.find(w);
                if (r1 != r2) {
                    T.add(e);
                    uf.union(r1, r2);
                    counter++;
                }
            }
        }

        public void makeTree() {
            for(Edge e: T) {
                Tree.addEdge(e);
            }
        }

        public int bandwidth(int s, int t) {
            for(int i = 0; i < 5000; i++) {
                marked[i] = false;
                bw[i] = 0;
            }
            bw[s] = 100000;
            dfs(s);
            return bw[t];
        }

        private void dfs(int s) {
            marked[s] = true;
            for (Edge w : Tree.adj(s)) {
                if (!marked[w.other(s)]) {
                    //edgeTo[w.other(s)] = s;
                    int tmp = Integer.min(bw[s], w.weight);
                    if(bw[w.other(s)]==0)
                        bw[w.other(s)] = tmp;
                    else
                        bw[w.other(s)] = Integer.max(bw[w.other(s)], tmp);
                    dfs(w.other(s));
                }
            }
        }

    }

    public class MaxHeap {
        private node[] mh;
        private int N;
        private int[] vi;

        public MaxHeap(int capacity) {
            mh = new node[capacity + 1];
            N = 0;
            vi = new int[capacity];
        }

        private class node {
            int vertex, value;
            public node(int v1, int v2) {
                vertex = v1;
                value = v2;
            }
        }

        public void insert(int v, int val) {
            N++;
            mh[N] = new node(v,val);
            vi[v] = N;
            swim(N);
        }

        public boolean isEmpty() { return N == 0; }

        public int size() {
            return N;
        }

        public int maxIndex() {
            if (N == 0) throw new NoSuchElementException("MaxHeap underflow");
            return mh[1].vertex;
        }

        public int maxValue() {
            if (N == 0) throw new NoSuchElementException("MaxHeap underflow");
            return mh[1].value;
        }

        public void delMax() {
            if (N == 0) throw new NoSuchElementException("MaxHeap underflow");
            exch(1, N--);
            sink(1);
            mh[N+1] = null;
        }

        public void delete(int i) {
            if (N == 0) throw new NoSuchElementException("MaxHeap underflow");
            int index = 1;
            while(index <= N) {
                if(mh[index].vertex == i)
                    break;
                else
                    index++;
            }
            exch(index, N--);
            sink(index);
            mh[N+1] = null;
        }

        public void update(int v, int val) {
            if (N == 0) throw new NoSuchElementException("MaxHeap underflow");
            int index = vi[v];
            mh[index].value = val;
            swim(index);
        }

        private void swim(int k)  {
            while (k > 1 && less(k/2,k)) {
                exch(k, k/2);
                k = k/2;
            }
        }

        private void sink(int k) {
            while (2*k <= N) {
                int j = 2*k;
                if (j < N && less(j,j+1)) j++;
                if (!less(k,j)) break;
                exch(k, j);
                k = j;
            }
        }

        private boolean less(int i, int j) {
            return (mh[i].value - mh[j].value < 0);
        }

        private void exch(int i, int j) {
            node swap = mh[i];
            mh[i] = mh[j];
            mh[j] = swap;
            vi[mh[i].vertex] = i;
            vi[mh[j].vertex] = j;
        }

    }
}
