class Demo.HelloWorld : GLib.Object {

    public static int main(string[] args)
    {
        try
        {
            for (int deal = 1; deal <= 1000000 ; deal++)
            {
                if (deal % 1000 == 0)
                {
                    stderr.printf("Reached %d\n", deal);
                }
                string fn = deal.to_string() + ".rs";
                // stdout.printf("%s\n", fn);
                
                var file = File.new_for_path(fn);

                if (file.query_exists(null))
                {
                    var in_stream = new DataInputStream (file.read (null));
                    string line = in_stream.read_line(null,null);

                    if (line != null)
                    {
                        if (/^Intract/.match(line))
                        {
                            stdout.printf("%s is intractable.\n", fn);
                        }
                    }
                    else
                    {
                        stdout.printf("%s is empty.\n", fn);
                    }
                }
                else
                {
                    stdout.printf("%s is missing.\n", fn);
                }
            }
        }
        catch (Error e) 
        {
            error ("%s", e.message);
        }

        return 0;
    }
}
