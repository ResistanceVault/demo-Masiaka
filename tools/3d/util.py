from math import pi

def format_number(n, accuracy=6):
    """Formats a number in a friendly manner (removes trailing zeros and unneccesary point."""
   
    fs = "%."+str(accuracy)+"f"
    str_n = fs%float(n)
    if '.' in str_n:
        str_n = str_n.rstrip('0').rstrip('.')
    if str_n == "-0":
        str_n = "0"
    #str_n = str_n.replace("-0", "0")
    return str_n
   

def lerp(a, b, i):
    """Linear enterpolate from a to b."""
    return a+(b-a)*i