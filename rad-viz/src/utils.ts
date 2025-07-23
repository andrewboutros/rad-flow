export const clearElement = (element: HTMLElement | null): void => {
  element && element.childNodes.forEach((node) => element.removeChild(node));
};

export const getDrawIOSvgString = (xml: XMLDocument) => {
  return xmlToString(xml.documentElement.firstChild?.firstChild || null);
};

export const xmlToString = (xml: Node | null): string | null => {
  if (!xml) return null;
  try {
    const serialize = new XMLSerializer();
    return serialize.serializeToString(xml);
  } catch (error) {
    console.log("XmlToString Error: ", error);
    return null;
  }
};

export const stringToXml = (str: string): XMLDocument | null => {
  try {
    const parser = new DOMParser();
    return parser.parseFromString(str, "text/xml") as XMLDocument;
  } catch (error) {
    console.log("StringToXml Error: ", error);
    return null;
  }
};

export const svgToString = (svg: Node | null): string | null => {
  if (!svg) return null;
  try {
    const serialize = new XMLSerializer();
    return serialize.serializeToString(svg);
  } catch (error) {
    console.log("SvgToString Error: ", error);
    return null;
  }
};

export const stringToSvg = (str: string): SVGElement | null => {
  try {
    const parser = new DOMParser();
    return parser.parseFromString(str, "image/svg+xml")
      .firstChild as SVGElement;
  } catch (error) {
    console.log("StringToSvg Error: ", error);
    return null;
  }
};

export const base64ToSvgString = (base64: string): string | null => {
  try {
    const svg = atob(base64.replace("data:image/svg+xml;base64,", ""));
    return svg;
  } catch (error) {
    console.log("base64ToSvgString Error: ", error);
    return null;
  }
};

import pako from "pako";

export const stripXmlString = (str: string): string => {
  const node = stringToXml(str)!.documentElement;
  if (node != null && node.nodeName == "mxfile") {
    // Only select the first page and drop all other existing pages
    const diagrams = node.getElementsByTagName("diagram");
    if (diagrams[0].children.length >= 1) {
      str = xmlToString(diagrams[0].children[0])!; // has already been decoded
    } else {
      str = diagrams[0].textContent!; // has not been decoded yet
    }
  }
  return str;
};

export const decodeDiagramXmlString = (data: string): string => {
  data = stripXmlString(data);
  if (data.length > 0) {
    data = atob(data);
    data = pako.inflateRaw(
      Uint8Array.from(data, (c) => c.charCodeAt(0)),
      { to: "string" }
    );
    data = decodeURIComponent(data);
  }
  return data;
};

export const getContrastFontColor = (bgColor: string): string => {
  bgColor = bgColor.substring(1);
  const red = parseInt(bgColor.substring(0, 2), 16);
  const green = parseInt(bgColor.substring(2, 4), 16);
  const blue = parseInt(bgColor.substring(4, 6), 16);
  const brightness = red * 0.299 + green * 0.587 + blue * 0.114;
  /* if (red*0.299 + green*0.587 + blue*0.114) > 180
   * use #000000 else use #ffffff
   * reference: https://www.w3.org/TR/AERT/#color-contrast
   */
  return brightness > 180 ? "#000000" : "#ffffff";
};
