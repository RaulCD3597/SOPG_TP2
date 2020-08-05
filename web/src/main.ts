class Model{
    public l:string[]=["0","0","0","0","0","0"];

    loadStates(data):void{
        this.l[0] = data.lines[0];
        this.l[1] = data.lines[1];
        this.l[2] = data.lines[2];
        this.l[3] = data.lines[3];
        this.l[4] = data.lines[4];
        this.l[5] = data.lines[5];
    }

    getLineStates() {
        let d = {"l0":this.l[0],"l1":this.l[1],"l2":this.l[2],"l3":this.l[3],"l4":this.l[4],"l5":this.l[5]};
        return d;
    }


    toggleState(lineNumber:number):void{
        switch(this.l[lineNumber])
        {
            case "0": this.l[lineNumber]="1";break;
            case "1": this.l[lineNumber]="2";break;
            case "2": this.l[lineNumber]="0";break;
        }       
    }
}
class ViewMainPage
{
    myf:MyFramework;
    model:Model;
    flagToggle:boolean = false;

    constructor(myf:MyFramework,m:Model)
    {
        this.myf = myf;    
        this.model = m;

        setInterval(()=>{this.updateImages()},250);
        setInterval(()=>{this.blink()},600);
    }

    blink():void {
        if(this.flagToggle)
            this.flagToggle=false;
        else
            this.flagToggle=true;
    }

    changeImg(el,state,lineNumber):void {
        switch(state)
        {
            case "0":el.src="images/"+lineNumber+"_on.png";break;
            case "1":el.src="images/"+lineNumber+"_off.png";break;
            case "2":
                if(this.flagToggle)
                    el.src="images/"+lineNumber+"_on.png";
                else
                    el.src="images/"+lineNumber+"_blank.png";
                break;
        }    
    }

    updateImages():void {

        let el:HTMLElement = this.myf.getElementById("line_0");
        this.changeImg(el,this.model.l[0],0);

        el = this.myf.getElementById("line_1");
        this.changeImg(el,this.model.l[1],1);

        el = this.myf.getElementById("line_2");
        this.changeImg(el,this.model.l[2],2);

        el = this.myf.getElementById("line_3");
        this.changeImg(el,this.model.l[3],3);

        el = this.myf.getElementById("line_4");
        this.changeImg(el,this.model.l[4],4);

        el = this.myf.getElementById("line_5");
        this.changeImg(el,this.model.l[5],5);
    }

}
class Main implements GETResponseListener, EventListenerObject, POSTResponseListener
{ 
    myf:MyFramework;
    view:ViewMainPage;
    model:Model;

    handleEvent(evt:Event):void
    {
        let el: HTMLElement = this.myf.getElementByEvent(evt);
        console.log("click en elemento:"+el.id);
        if(el.id=="btnMsg")
        {

        }
        else
        {
            // click en imagen
            let lineNumber:number = parseInt( el.id.split("_")[1] );
            console.log(lineNumber);

            this.model.toggleState(lineNumber);

            let d = this.model.getLineStates();
            //let data:object = {"data":d};
            console.log(d);
            this.myf.requestPOST("cgi-bin/setStates.py",d,this);
        }

    }

    handleGETResponse(status:number,response:string):void{
      if(status==200)
      {
          let data = JSON.parse(response);
          //this.view.showDevices(data);    
            console.log(data);
            this.model.loadStates(data);
      }
    }

    handlePOSTResponse(status:number,response:string):void{
        if(status==200)
        {
            console.log(response);
        }
    }

    main():void 
    { 
      this.myf = new MyFramework();
      this.model = new Model();

      this.view = new ViewMainPage(this.myf,this.model);

      setInterval(()=>{this.myf.requestGET("cgi-bin/getLines.py",this)},1000);

      let i:number;
      for(i=0; i<6;i++)
      {
        let im:HTMLElement = this.myf.getElementById("line_"+i);
        im.addEventListener("click",this);                
      }
      let b:HTMLElement = this.myf.getElementById("btnMsg");
      b.addEventListener("click",this);                

    } 
} 
 
window.onload = () => {
    let obj = new Main(); 
    obj.main();
};
 

