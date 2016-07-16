import java.io.*;

public class Vote {

    public void vote() throws IOException{
	int v = getVote();
	if (v != -1){
	    submit(v);
	} else {
	    vote();
	}
    }

    public int getVote() throws IOException{
	BufferedReader br = new BufferedReader(
		       new InputStreamReader(System.in));
	int v = br.read() - 48;
	if (validate(v)) {
	    return v;
	} else{
	    return -1;
	}
    }

    public boolean validate(int v){
	return ((1 <= v) &&(v <= 5));
    }

    public void submit(int v){
	System.out.printf("The vote %d is submitted!", v);
    }

    public static void main(String[] args) {
        Vote voteobj = new Vote();
        try{
	    voteobj.vote();
        } catch(Exception ie){
            System.out.print(ie.toString());
        }
    }
}
