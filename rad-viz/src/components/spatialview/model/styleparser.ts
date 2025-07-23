interface MxStyleItem {
  attr: string;
  existing: boolean;
  defaultAttr: string; // for resetting style
}

export default class MxGraphStyle {
  protected styleItems: { [name: string]: MxStyleItem };

  protected styleString: string;
  protected matchItem(pattern: RegExp, defaultAttr: string): MxStyleItem {
    const match = this.styleString.match(pattern);
    if (match === null) {
      return { attr: defaultAttr, existing: false, defaultAttr: defaultAttr };
    } else {
      return { attr: match[1], existing: true, defaultAttr: defaultAttr };
    }
  }

  protected maintainStyleItem(
    name: string,
    defaultAttr: string /*use only when no such attr exists */
  ) {
    const pattern = new RegExp(`${name}=(.*?);`);
    this.styleItems[name] = this.matchItem(pattern, defaultAttr);
  }

  protected replaceStyleItem(name: string, newAttr: string) {
    const pattern = new RegExp(`${name}=(.*?);`);
    this.styleString = this.styleString.replace(pattern, `${name}=${newAttr};`);
  }

  constructor(rawStyleString: string | null) {
    this.styleString = rawStyleString === null ? "" : rawStyleString;
    this.styleItems = {};
    this.maintainStyleItem("strokeColor", "#0000" /*grey*/);
    this.maintainStyleItem("strokeWidth", "1");
  }

  setStyleAttribute(key: string, value: string) {
    if (key in this.styleItems) {
      this.styleItems[key].attr = value;
    } else {
      // console.log(
      //   `${key} is not found in the MxGraphStyle instance, so start to maintain this one`
      // );
      this.maintainStyleItem(key, "DontCare");
      this.styleItems[key].attr = value; // force using new value
    }
  }

  resetStyle() {
    Object.values(this.styleItems).forEach((item) => {
      item.attr = item.defaultAttr;
    });
  }

  getStyleString() {
    Object.entries(this.styleItems).forEach(([name, item]) => {
      // console.log(name, item);
      if (item.existing) {
        this.replaceStyleItem(name, item.attr);
      } else {
        if (
          this.styleString.length >= 1 &&
          this.styleString[this.styleString.length - 1] !== ";"
        ) {
          this.styleString += ";";
        }
        this.styleString += `${name}=${item.attr};`;
        item.existing = true;
      }
    });
    return this.styleString;
  }
}

// Simple test:
// const mxStyle = new MxGraphStyle("rounded=0;strokeWidth=1;shadow=0;")
// mxStyle.setAttributes("rounded", "1");
// console.log(mxStyle.getStyleString());
